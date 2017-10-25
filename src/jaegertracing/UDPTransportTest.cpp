/*
 * Copyright (c) 2017 Uber Technologies, Inc.
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
#include "jaegertracing/testutils/MockAgent.h"
#include "jaegertracing/utils/ErrorUtil.h"

namespace jaegertracing {

TEST(UDPTransport, testManyMessages)
{
    auto mockAgent = testutils::MockAgent::make();
    mockAgent->start();
    std::ostringstream samplingServerURLStream;
    samplingServerURLStream << "http://"
                            << mockAgent->samplingServerAddr().authority();

    Config config(false,
                  samplers::Config("const",
                                   1,
                                   samplingServerURLStream.str(),
                                   0,
                                   samplers::Config::Clock::duration()),
                  reporters::Config(0,
                                    reporters::Config::Clock::duration(),
                                    false,
                                    mockAgent->spanServerAddress().authority()),
                  propagation::HeadersConfig(),
                  baggage::RestrictionsConfig());
    auto tracer = Tracer::make("test-service",
                               config,
                               logging::consoleLogger());

    UDPTransport sender(mockAgent->spanServerAddress(), 0);
    constexpr auto kNumMessages = 2000;
    const auto logger = logging::consoleLogger();
    logger->set_level(spdlog::level::info);
    for (auto i = 0; i < kNumMessages; ++i) {
        Span span(std::static_pointer_cast<const Tracer>(tracer));
        span.SetOperationName("test" + std::to_string(i));
        ASSERT_NO_THROW(sender.append(span));
    }
}

}  // namespace jaegertracing
