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

#include "jaegertracing/SpanContext.h"
#include "jaegertracing/TraceID.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <sstream>
#include <string>

namespace jaegertracing {
namespace {

using PropagationFormat = propagation::Format;

struct FromStreamTestCase {
    std::string _input;
    bool _success;
};

}  // anonymous namespace

TEST(SpanContext, testFromStreamWithJaegerFormat)
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
        SpanContext spanContext;
        {
            std::stringstream ss;
            ss << testCase._input;
            spanContext = SpanContext::fromStream(ss, PropagationFormat::JAEGER);
            ASSERT_EQ(testCase._success, spanContext.isValid())
                << "input=" << testCase._input;
        }

        SpanContext spanContextFromStreamOp;
        {
            std::stringstream ss;
            ss << testCase._input;
            ss >> spanContextFromStreamOp;
        }

        ASSERT_EQ(spanContext, spanContextFromStreamOp);
    }
}

TEST(SpanContext, testFromStreamWithW3CFormat)
{
    const FromStreamTestCase testCases[] = {
        { "00-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", true },
        { "00-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-00", true },
        { "00-11111111111111111111111111111111-2222222222222222-01", true },
        { "00-AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA-BBBBBBBBBBBBBBBB-01", true },
        { "", false },
        { "000af7651916cd43dd8448eb211c80319cb9c7c989f97918e101", false },
        { "000af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "00-0af7651916cd43dd8448eb211c80319cb9c7c989f97918e1-01", false },
        { "00-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e101", false },
        { "000af7651916cd43dd8448eb211c80319cb9c7c989f97918e1-01", false },
        { "00-0af7651916cd43dd8448eb211c80319cb9c7c989f97918e101", false },
        { "01-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "0-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "000-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "0k-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "k0-0af7651916cd43dd8448eb211c80319c-b9c7c989f97918e1-01", false },
        { "00-00000000000000000000000000000000-b9c7c989f97918e1-01", false },
        { "01-0af7651916cd43dd8448eb211c80319-b9c7c989f97918e1-01", false },
        { "01-0af7651916cd43dd8448eb211c80319cc-b9c7c989f97918e1-01", false },
        { "00-0af7651916cd43dd8448eb211c80319c-0000000000000000-01", false }
    };

    for (auto&& testCase : testCases) {
        SpanContext spanContext;
        {
            std::stringstream ss;
            ss << testCase._input;
            spanContext = SpanContext::fromStream(ss, PropagationFormat::W3C);
            ASSERT_EQ(testCase._success, spanContext.isValid())
                << "input=" << testCase._input;
        }
    }
}

TEST(SpanContext, testJaegerFormatting)
{
    SpanContext spanContext(TraceID(255, 1), 2, 3, 1, SpanContext::StrMap());
    {
        std::ostringstream oss;
        spanContext.print(oss, PropagationFormat::JAEGER);
        ASSERT_EQ("ff0000000000000001:2:3:1", oss.str());
    }
    {
        std::ostringstream oss;
        oss << spanContext;
        ASSERT_EQ("ff0000000000000001:2:3:1", oss.str());
    }
}

TEST(SpanContext, testW3CFormatting)
{
    SpanContext spanContext(TraceID(255, 1), 2, 3, 1, SpanContext::StrMap());
    std::ostringstream oss;
    spanContext.print(oss, PropagationFormat::W3C);
    ASSERT_EQ("00-00000000000000ff0000000000000001-0000000000000002-01",
              oss.str());
}

TEST(SpanContext, testBaggage)
{
    const SpanContext spanContext(
        TraceID(0, 0),
        0,
        0,
        0,
        SpanContext::StrMap({ { "key1", "value1" }, { "key2", "value2" } }));
    std::string keyCopy;
    std::string valueCopy;
    spanContext.ForeachBaggageItem(
        [&keyCopy, &valueCopy](const std::string& key,
                               const std::string& value) {
            keyCopy = key;
            valueCopy = value;
            return false;
        });
    ASSERT_TRUE(keyCopy == "key1" || keyCopy == "key2");
    if (keyCopy == "key1") {
        ASSERT_EQ("value1", valueCopy);
    }
    else {
        ASSERT_EQ("value2", valueCopy);
    }
}

TEST(SpanContext, testDebug)
{
    const SpanContext spanContext;
    ASSERT_FALSE(spanContext.isDebug());
    ASSERT_FALSE(spanContext.isDebugIDContainerOnly());
}

}  // namespace jaegertracing
