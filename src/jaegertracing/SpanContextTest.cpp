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

#include <sstream>

#include "jaegertracing/SpanContext.h"

namespace jaegertracing {
namespace {

struct FromStreamTestCase {
    std::string _input;
    bool _success;
};

}  // anonymous namespace

TEST(SpanContext, testFromStream)
{
    const FromStreamTestCase testCases[] = {
        { "", false },
        { "abcd", false },
        { "ABCD", false },
        { "x:1:1:1", false },
        { "1:x:1:1", false },
        { "1:1:x:1", false },
        { "1:1:1:x", false },
        { "01234567890123456789012345678901234:1:1:1", false },
        { "01234567890123456789012345678901:1:1:1", true },
        { "01234_67890123456789012345678901:1:1:1", false },
        { "0123456789012345678901_345678901:1:1:1", false },
        { "1:0123456789012345:1:1", true },
        { "1:01234567890123456:1:1", false },
        { "10000000000000001:1:1:1", true },
        { "10000000000000001:1:1", false },
        { "1:1:1:1", true }
    };

    for (auto&& testCase : testCases) {
        std::stringstream ss;
        ss << testCase._input;
        auto spanContext = SpanContext::fromStream(ss);
        ASSERT_EQ(testCase._success, spanContext.isValid())
            << "input=" << testCase._input;
    }
}

TEST(SpanContext, testFormatting)
{
    SpanContext spanContext(TraceID(255, 255), 0, 0, 0, SpanContext::StrMap());
    std::ostringstream oss;
    oss << spanContext;
    ASSERT_EQ("ff00000000000000ff:0:0:0", oss.str());
}

}  // namespace jaegertracing
