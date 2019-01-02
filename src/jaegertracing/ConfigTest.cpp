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

#include "jaegertracing/Config.h"
#include "jaegertracing/Constants.h"
#include "jaegertracing/propagation/HeadersConfig.h"
#include "jaegertracing/samplers/Config.h"
#include "jaegertracing/utils/YAML.h"
#include <gtest/gtest.h>

namespace jaegertracing {

#ifdef JAEGERTRACING_WITH_YAML_CPP

TEST(Config, testParse)
{
    {
        constexpr auto kConfigYAML = R"cfg(
disabled: true
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
    traceBaggageHeaderPrefix: "testctx-"
baggage_restrictions:
    denyBaggageOnInitializationFailure: false
    hostPort: 127.0.0.1:5778
    refreshInterval: 60
)cfg";
        const auto config = Config::parse(YAML::Load(kConfigYAML));
        ASSERT_EQ("probabilistic", config.sampler().type());
        ASSERT_EQ("debug-id", config.headers().jaegerDebugHeader());
        ASSERT_EQ("baggage", config.headers().jaegerBaggageHeader());
        ASSERT_EQ("trace-id", config.headers().traceContextHeaderName());
        ASSERT_EQ("testctx-", config.headers().traceBaggageHeaderPrefix());
    }

    {
        Config::parse(YAML::Load(R"cfg(
disabled: false
sampler: 1
reporter: 2
headers: 3
baggage_restrictions: 4
)cfg"));
    }
}

TEST(Config, testDefaultSamplingProbability)
{
    ASSERT_EQ(samplers::Config::kDefaultSamplingProbability,
              Config().sampler().param());
}

TEST(Config, testZeroSamplingParam)
{
    {
        constexpr auto kConfigYAML = R"cfg(
sampler:
    param: 0
)cfg";
        const auto config = Config::parse(YAML::Load(kConfigYAML));
        ASSERT_EQ(0, config.sampler().param());
    }
}

#endif  // JAEGERTRACING_WITH_YAML_CPP

}  // namespace jaegertracing
