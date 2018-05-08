/*
 * Copyright (c) 2018 Uber Technologies, Inc.
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
#include <iostream>
#include <string>

#include "jaegertracing/Constants.h"
#include <gtest/gtest.h>
#include <opentracing/dynamic_load.h>

namespace jaegertracing {
namespace {

void trace(opentracing::TracerFactory& factory)
{
    std::string errorMessage;
    auto result = factory.MakeTracer(R"(
service_name: test-service
disabled: false
sampler:
    type: probabilistic
    param: 0.001
reporter:
    queueSize: 100
    bufferFlushInterval: 10
    logSpans: false
    localAgentHostPort: 127.0.0.1:6831
headers:
    jaegerDebugHeader: debug-id
    jaegerBaggageHeader: baggage
    TraceContextHeaderName: trace-id
    traceBaggageHeaderPrefix: testctx-
baggage_restrictions:
    denyBaggageOnInitializationFailure: false
    hostPort: 127.0.0.1:5778
    refreshInterval: 60
)",
                                     errorMessage);
    assert(errorMessage.empty());
    auto tracer = static_cast<std::shared_ptr<opentracing::Tracer>>(*result);
    {
        tracer->StartSpan("a");
        std::cout << "Started span\n";
    }
    std::cout << "Finished span\n";
}

}  // anonymous namespace

#ifdef JAEGERTRACING_WITH_YAML_CPP
TEST(DynamicLoad, invalidVersion)
{
    const void* errorCategory = nullptr;
    void* tracerFactory = nullptr;
    const auto rcode = OpenTracingMakeTracerFactory(
        "0.0.0" /*invalid version*/, &errorCategory, &tracerFactory);
    ASSERT_EQ(rcode, opentracing::incompatible_library_versions_error.value());
    ASSERT_EQ(
        errorCategory,
        static_cast<const void*>(&opentracing::dynamic_load_error_category()));
    ASSERT_EQ(tracerFactory, nullptr);
}

TEST(DynamicLoad, validVersion)
{
    const void* errorCategory = nullptr;
    void* tracerFactory = nullptr;
    const auto rcode = OpenTracingMakeTracerFactory(
        OPENTRACING_VERSION, &errorCategory, &tracerFactory);
    ASSERT_EQ(rcode, 0);
    ASSERT_EQ(errorCategory, nullptr);
    ASSERT_NE(tracerFactory, nullptr);
    std::unique_ptr<opentracing::TracerFactory> factory(
        static_cast<opentracing::TracerFactory*>(tracerFactory));
    trace(*factory);
}
#else
TEST(DynamicLoad, noYAML)
{
    const void* errorCategory = nullptr;
    void* tracerFactory = nullptr;
    const auto rcode = OpenTracingMakeTracerFactory(
        OPENTRACING_VERSION, &errorCategory, &tracerFactory);
    ASSERT_NE(rcode, 0);
}
#endif  // JAEGERTRACING_WITH_YAML_CPP
}  // namespace jaegertracing
