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
#include <iostream>
#include <string>

#include <gtest/gtest.h>
#include <opentracing/dynamic_load.h>

namespace jaegertracing {
TEST(DynamicLoad, invalidVersion)
{
    const void* errorCategory = nullptr;
    void* tracerFactory = nullptr;
    const auto rcode = OpenTracingMakeTracerFactory(
        "1.0.0" /*invalid version*/, &errorCategory, &tracerFactory);
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
    delete static_cast<opentracing::TracerFactory*>(tracerFactory);
}
}  // namespace jaegertracing
