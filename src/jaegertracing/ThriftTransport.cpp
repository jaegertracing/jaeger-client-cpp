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

#include "jaegertracing/ThriftTransport.h"

#include "jaegertracing/Span.h"
#include "jaegertracing/Tag.h"
#include "jaegertracing/Tracer.h"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <iterator>
#include <thrift/transport/TBufferTransports.h>

#ifdef _MSC_VER
#pragma warning(disable : 4267)  // Conversion from unsigned to signed. It
                                 // should not be a problem here.
#endif

namespace jaegertracing {
namespace net {
class IPAddress;
}  // namespace net

namespace {

constexpr auto kEmitBatchOverhead = 30;

template <typename ThriftType>
int calcSizeOfSerializedThrift(apache::thrift::protocol::TProtocolFactory& factory, const ThriftType& base, int maxPacketSize)
{
    std::shared_ptr<apache::thrift::transport::TMemoryBuffer> buffer(
        new apache::thrift::transport::TMemoryBuffer(maxPacketSize));
    auto protocol = factory.getProtocol(buffer);
    base.write(protocol.get());
    uint8_t* data = nullptr;
    uint32_t size = 0;
    buffer->getBuffer(&data, &size);
    return size;
}

}  // anonymous namespace

ThriftTransport::ThriftTransport(const net::IPAddress& ip,
                                 int maxPacketSize)
    : _sender(new utils::UDPSender(ip, maxPacketSize))
    , _maxSpanBytes(0)
    , _byteBufferSize(0)
    , _processByteSize(0)
    , _protocolFactory(_sender->protocolFactory())
{
}

ThriftTransport::ThriftTransport(const net::URI& endpoint,
                                 int maxPacketSize)
    : _sender(new utils::HttpSender(endpoint, maxPacketSize))
    , _maxSpanBytes(0)
    , _byteBufferSize(0)
    , _processByteSize(0)
    , _protocolFactory(_sender->protocolFactory())
{
}


int ThriftTransport::append(const Span& span)
{
    if (_process.serviceName.empty()) {
        const auto& tracer = static_cast<const Tracer&>(span.tracer());
        _process.serviceName = tracer.serviceName();

        const auto& tracerTags = tracer.tags();
        std::vector<thrift::Tag> thriftTags;
        thriftTags.reserve(tracerTags.size());
        std::transform(std::begin(tracerTags),
                       std::end(tracerTags),
                       std::back_inserter(thriftTags),
                       [](const Tag& tag) {
                           thrift::Tag thriftTag;
                           tag.thrift(thriftTag);
                           return thriftTag;
                       });
        _process.__set_tags(thriftTags);

        _processByteSize =
            calcSizeOfSerializedThrift(*_protocolFactory, _process, _sender->maxPacketSize());
        _maxSpanBytes =
            _sender->maxPacketSize() - _processByteSize - kEmitBatchOverhead;
    }
    thrift::Span jaegerSpan;
    span.thrift(jaegerSpan);
    const auto spanSize =
        calcSizeOfSerializedThrift(*_protocolFactory, jaegerSpan, _sender->maxPacketSize());
    if (spanSize > _maxSpanBytes) {
        std::ostringstream oss;
        throw Transport::Exception("Span is too large", 1);
    }

    _byteBufferSize += spanSize;
    if (_byteBufferSize <= _maxSpanBytes) {
        _spanBuffer.push_back(jaegerSpan);
        if (_byteBufferSize < _maxSpanBytes) {
            return 0;
        }
        return flush();
    }

    // Flush currently full buffer, then append this span to buffer.
    const auto flushed = flush();
    _spanBuffer.push_back(jaegerSpan);
    _byteBufferSize = spanSize + _processByteSize;
    return flushed;
}

int ThriftTransport::flush()
{
    if (_spanBuffer.empty()) {
        return 0;
    }

    thrift::Batch batch;
    batch.__set_process(_process);
    batch.__set_spans(_spanBuffer);

    try {
        _sender->emitBatch(batch);
    } catch (const std::system_error& ex) {
        std::ostringstream oss;
        oss << "Could not send span " << ex.what()
            << ", code=" << ex.code().value();
        throw Transport::Exception(oss.str(), _spanBuffer.size());
    } catch (const std::exception& ex) {
        std::ostringstream oss;
        oss << "Could not send span " << ex.what();
        throw Transport::Exception(oss.str(), _spanBuffer.size());
    } catch (...) {
        throw Transport::Exception("Could not send span, unknown error",
                                   _spanBuffer.size());
    }

    resetBuffers();

    return batch.spans.size();
}

}  // namespace jaegertracing
