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

#include <gtest/gtest.h>

#include "jaegertracing/Config.h"
#include "jaegertracing/Tracer.h"
#include "jaegertracing/UDPTransport.h"
#include "jaegertracing/testutils/TracerUtil.h"
#include "jaegertracing/utils/ErrorUtil.h"

namespace jaegertracing {
namespace {

class MockUDPClient : public utils::UDPClient {
  public:
    enum class ExceptionType { kSystemError, kException, kString };

    MockUDPClient(const net::IPAddress& serverAddr,
                  int maxPacketSize,
                  ExceptionType type)
        : UDPClient(serverAddr, maxPacketSize)
        , _type(type)
    {
    }

  private:
    void emitBatch(const thrift::Batch& batch) override
    {
        switch (_type) {
        case ExceptionType::kSystemError:
            throw std::system_error(
                std::make_error_code(std::errc::invalid_argument));
        case ExceptionType::kException:
            throw std::exception();
        default:
            assert(_type == ExceptionType::kString);
            throw "error";
        }
    }

    ExceptionType _type;
};

class MockUDPTransport : public UDPTransport {
  public:
    MockUDPTransport(const net::IPAddress& ip,
                     int maxPacketSize,
                     MockUDPClient::ExceptionType type)
        : UDPTransport(ip, maxPacketSize)
    {
        setClient(std::unique_ptr<utils::UDPClient>(
            new MockUDPClient(ip, maxPacketSize, type)));
    }
};

}  // anonymous namespace

TEST(UDPTransport, testManyMessages)
{
    const auto handle = testutils::TracerUtil::installGlobalTracer();
    const auto tracer =
        std::static_pointer_cast<const Tracer>(opentracing::Tracer::Global());

    UDPTransport sender(handle->_mockAgent->spanServerAddress(), 0);
    constexpr auto kNumMessages = 2000;
    const auto logger = logging::consoleLogger();
    for (auto i = 0; i < kNumMessages; ++i) {
        Span span(tracer);
        span.SetOperationName("test" + std::to_string(i));
        ASSERT_NO_THROW(sender.append(span));
    }
}

TEST(UDPTransport, testExceptions)
{
    const auto handle = testutils::TracerUtil::installGlobalTracer();
    const auto tracer =
        std::static_pointer_cast<const Tracer>(opentracing::Tracer::Global());

    Span span(tracer);
    span.SetOperationName("test");

    const MockUDPClient::ExceptionType exceptionTypes[] = {
        MockUDPClient::ExceptionType::kSystemError,
        MockUDPClient::ExceptionType::kException,
        MockUDPClient::ExceptionType::kString
    };
    for (auto type : exceptionTypes) {
        MockUDPTransport sender(net::IPAddress(), 0, type);
        sender.append(span);
        ASSERT_THROW(sender.flush(), Transport::Exception);
    }
}

}  // namespace jaegertracing
