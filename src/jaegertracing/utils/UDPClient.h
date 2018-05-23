/*
 * Copyright (c) 2017-2018 Uber Technologies, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef JAEGERTRACING_UTILS_UDPCLIENT_H
#define JAEGERTRACING_UTILS_UDPCLIENT_H

#include <iostream>
#include <sstream>
#include <stdexcept>
#include <system_error>

#include <thrift/protocol/TCompactProtocol.h>
#include <thrift/transport/TBufferTransports.h>

#include "jaegertracing/net/IPAddress.h"
#include "jaegertracing/net/Socket.h"
#include "jaegertracing/thrift-gen/Agent.h"

namespace jaegertracing {
namespace utils {

class UDPClient : public agent::thrift::AgentIf {
  public:
    UDPClient(const net::IPAddress& serverAddr, int maxPacketSize);

    ~UDPClient() { close(); }

    void emitZipkinBatch(
        const std::vector<twitter::zipkin::thrift::Span>& spans) override
    {
        throw std::logic_error("emitZipkinBatch not implemented");
    }

    void emitBatch(const thrift::Batch& batch) override
    {
        _buffer->resetBuffer();
        _client->emitBatch(batch);
        uint8_t* data = nullptr;
        uint32_t size = 0;
        _buffer->getBuffer(&data, &size);
        if (static_cast<int>(size) > _maxPacketSize) {
            std::ostringstream oss;
            oss << "Data does not fit within one UDP packet"
                   ", size "
                << size << ", max " << _maxPacketSize << ", spans "
                << batch.spans.size();
            throw std::logic_error(oss.str());
        }
        const auto numWritten = ::write(_socket.handle(), data, size);
        if (static_cast<unsigned>(numWritten) != size) {
            std::ostringstream oss;
            oss << "Failed to write message"
                   ", numWritten="
                << numWritten << ", size=" << size;
            throw std::system_error(errno, std::system_category(), oss.str());
        }
    }

    int maxPacketSize() const { return _maxPacketSize; }

    void close() { _socket.close(); }

  private:
    int _maxPacketSize;
    std::shared_ptr<apache::thrift::transport::TMemoryBuffer> _buffer;
    net::Socket _socket;
    net::IPAddress _serverAddr;
    std::unique_ptr<agent::thrift::AgentClient> _client;
};

}  // namespace utils
}  // namespace jaegertracing

#endif  // JAEGERTRACING_UTILS_UDPCLIENT_H
