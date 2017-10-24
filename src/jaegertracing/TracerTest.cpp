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

#include <sstream>

#include <gtest/gtest.h>

#include "jaegertracing/Tracer.h"
#include "jaegertracing/testutils/MockAgent.h"

namespace jaegertracing {

TEST(Tracer, testTracer)
{
    auto mockAgent = testutils::MockAgent::make();
    mockAgent->start();
    std::ostringstream samplingServerURLStream;
    samplingServerURLStream << "http://"
                            << mockAgent->samplingServerAddr().authority();
    Config config(false,
                  samplers::Config("",
                                   0,
                                   samplingServerURLStream.str(),
                                   0,
                                   samplers::Config::Clock::duration()),
                  reporters::Config(0,
                                    reporters::Config::Clock::duration(),
                                    false,
                                    mockAgent->spanServerAddress().authority()),
                  propagation::HeadersConfig(),
                  false,
                  baggage::RestrictionsConfig());
    auto tracer = Tracer::make("test-service",
                               config,
                               logging::consoleLogger());
    opentracing::Tracer::InitGlobal(tracer);
    std::unique_ptr<Span> span(static_cast<Span*>(
        tracer->StartSpanWithOptions("test-operation", {}).release()));
    ASSERT_TRUE(static_cast<bool>(span));
    ASSERT_EQ(static_cast<opentracing::Tracer*>(tracer.get()), &span->tracer());
    span->SetOperationName("test-set-operation");
    span->SetTag("tag-key", "tag-value");
    span->SetBaggageItem("test-baggage-item-key", "test-baggage-item-value");
    ASSERT_EQ("test-baggage-item-value",
              span->BaggageItem("test-baggage-item-key"));
    span->Log({ { "log-bool", true } });
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    span->Finish();
    ASSERT_GE(Span::Clock::now(), span->startTime() + span->duration());
    span->SetOperationName("test-set-operation-after-finish");
    ASSERT_EQ("test-set-operation", span->operationName());
    span->SetTag("tagged-after-finish-key", "tagged-after-finish-value");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    opentracing::Tracer::InitGlobal(opentracing::MakeNoopTracer());
}

}  // namespace jaegertracing
