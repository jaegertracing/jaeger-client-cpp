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

#ifndef JAEGERTRACING_THRIFTTRANSPORT_H
#define JAEGERTRACING_THRIFTTRANSPORT_H

#include "jaegertracing/Compilers.h"
#include "jaegertracing/Span.h"
#include "jaegertracing/Transport.h"
#include "jaegertracing/thrift-gen/jaeger_types.h"
#include "jaegertracing/utils/UDPSender.h"
#include "jaegertracing/utils/HttpSender.h"

namespace jaegertracing {

class ThriftTransport : public Transport {
  public:
    ThriftTransport(const net::IPAddress& ip, int maxPacketSize);

    ThriftTransport(const net::URI& endpoint, int maxPacketSize);

    ~ThriftTransport() { close(); }

    int append(const Span& span) override;

    int flush() override;

    void close() override { _sender->close(); }

  protected:
    void setClient(std::unique_ptr<utils::UDPSender>&& client)
    {
      _sender = std::move(client);
    }

  private:
    void resetBuffers()
    {
        _spanBuffer.clear();
        _byteBufferSize = _processByteSize;
    }

    std::unique_ptr<utils::Sender> _sender;
    int _maxSpanBytes;
    int _byteBufferSize;
    std::vector<thrift::Span> _spanBuffer;
    thrift::Process _process;
    int _processByteSize;
    std::unique_ptr<apache::thrift::protocol::TProtocolFactory> _protocolFactory;
};

}  // namespace jaegertracing

#endif  //JAEGERTRACING_THRIFTTRANSPORT_H
