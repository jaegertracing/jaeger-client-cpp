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

#include "jaegertracing/TraceID.h"
#include <gtest/gtest.h>
#include <sstream>

namespace jaegertracing {

using PropagationFormat = propagation::Format;

TEST(TraceID, testPrintJaegerFormat)
{
    std::ostringstream oss;
    TraceID(0, 10).print(oss, PropagationFormat::JAEGER);
    ASSERT_EQ("a", oss.str());
}

TEST(TraceID, testPrintW3CFormat)
{
    std::ostringstream oss;
    TraceID(0, 10).print(oss, PropagationFormat::W3C);
    ASSERT_EQ("0000000000000000000000000000000a", oss.str());
}

TEST(TraceID, testfromStreamJaegerFormat)
{
    std::stringstream ss;
    ss << "a";
    auto traceID = TraceID::fromStream(ss, PropagationFormat::JAEGER);
    ASSERT_EQ(TraceID(0, 10), traceID);
}

TEST(TraceID, testfromStreamW3CFormat)
{
    std::stringstream ss;
    ss << "0000000000000000000000000000000a";
    auto traceID = TraceID::fromStream(ss, PropagationFormat::JAEGER);
    ASSERT_EQ(TraceID(0, 10), traceID);
}

}  // namespace jaegertracing
