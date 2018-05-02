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

#ifndef JAEGERTRACING_UDPTRANSPORT_H
#define JAEGERTRACING_UDPTRANSPORT_H

#include "jaegertracing/Span.h"
#include "jaegertracing/Transport.h"
#include "jaegertracing/thrift-gen/jaeger_types.h"
#include "jaegertracing/utils/UDPClient.h"

namespace jaegertracing {

class UDPTransport : public Transport {
  public:
    UDPTransport(const net::IPAddress& ip, int maxPacketSize);

    ~UDPTransport() { close(); }

    int append(const Span& span) override;

    int flush() override;

    void close() override { _client->close(); }

  protected:
    void setClient(std::unique_ptr<utils::UDPClient>&& client)
    {
        _client = std::move(client);
    }

  private:
    void resetBuffers()
    {
        _spanBuffer.clear();
        _byteBufferSize = _processByteSize;
    }

    std::unique_ptr<utils::UDPClient> _client;
    int _maxSpanBytes;
    int _byteBufferSize;
    std::vector<thrift::Span> _spanBuffer;
    std::shared_ptr<apache::thrift::protocol::TProtocol> _protocol;
    thrift::Process _process;
    int _processByteSize;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_UDPTRANSPORT_H
