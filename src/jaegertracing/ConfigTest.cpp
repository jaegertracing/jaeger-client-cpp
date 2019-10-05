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

#include <cstdlib>

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

TEST(Config, testDefaultSamplingServerURL)
{
    ASSERT_EQ("http://127.0.0.1:5778/sampling",
              Config().sampler().samplingServerURL());
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


void setEnv(const char *variable, const char *value) {
#ifdef WIN32
  _putenv_s(variable, value);
#else
  setenv(variable, value, true);
#endif
}

TEST(Config, testFromEnv)
{
    setEnv(reporters::Config::kJAEGER_AGENT_HOST_ENV_PROP, "host33");
    setEnv(reporters::Config::kJAEGER_AGENT_PORT_ENV_PROP, "45");
    setEnv(reporters::Config::kJAEGER_ENDPOINT_ENV_PROP,
           "http://host34:56567");

    setEnv(reporters::Config::kJAEGER_REPORTER_MAX_QUEUE_SIZE_ENV_PROP,
           "33");
    setEnv(reporters::Config::kJAEGER_REPORTER_FLUSH_INTERVAL_ENV_PROP,
           "45");
    setEnv(
        reporters::Config::kJAEGER_REPORTER_LOG_SPANS_ENV_PROP, "true");

    setEnv(samplers::Config::kJAEGER_SAMPLER_PARAM_ENV_PROP, "33");
    setEnv(samplers::Config::kJAEGER_SAMPLER_TYPE_ENV_PROP, "const");
    setEnv(samplers::Config::kJAEGER_SAMPLER_MANAGER_HOST_PORT_ENV_PROP,
           "host34:56");

    setEnv(Config::kJAEGER_SERVICE_NAME_ENV_PROP, "AService");
    setEnv(Config::kJAEGER_TAGS_ENV_PROP,
           "hostname=foobar,my.app.version=1.2.3");

    Config config;

    config.fromEnv();

    ASSERT_EQ(std::string("http://host34:56567"), config.reporter().endpoint());
    ASSERT_EQ(std::string("host33:45"), config.reporter().localAgentHostPort());

    ASSERT_EQ(33, config.reporter().queueSize());
    ASSERT_EQ(std::chrono::milliseconds(45),
              config.reporter().bufferFlushInterval());
    ASSERT_EQ(true, config.reporter().logSpans());

    ASSERT_EQ(33., config.sampler().param());
    ASSERT_EQ(std::string("const"), config.sampler().type());
    ASSERT_EQ(std::string("http://host34:56/sampling"),
              config.sampler().samplingServerURL());

    ASSERT_EQ(std::string("AService"), config.serviceName());

    std::vector<Tag> expectedTags;
    expectedTags.emplace_back("hostname", std::string("foobar"));
    expectedTags.emplace_back("my.app.version", std::string("1.2.3"));
    ASSERT_EQ(expectedTags, config.tags());

    ASSERT_EQ(false, config.disabled());

    setEnv(Config::kJAEGER_JAEGER_DISABLED_ENV_PROP, "TRue");
    setEnv(reporters::Config::kJAEGER_AGENT_PORT_ENV_PROP, "445");

    config.fromEnv();
    ASSERT_EQ(true, config.disabled());
    ASSERT_EQ(std::string("host33:445"),
              config.reporter().localAgentHostPort());
}

}  // namespace jaegertracing
