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

#ifndef JAEGERTRACING_REPORTERS_CONFIG_H
#define JAEGERTRACING_REPORTERS_CONFIG_H

#include <chrono>
#include <memory>
#include <string>

#include "jaegertracing/Logging.h"
#include "jaegertracing/metrics/Metrics.h"
#include "jaegertracing/reporters/Reporter.h"
#include "jaegertracing/utils/YAML.h"

namespace jaegertracing {
namespace reporters {

class Config {
  public:
    using Clock = std::chrono::steady_clock;

    static constexpr auto kDefaultQueueSize = 100;
    static constexpr auto kDefaultLocalAgentHostPort = "127.0.0.1:6831";

    static Clock::duration defaultBufferFlushInterval()
    {
        return std::chrono::seconds(10);
    }

#ifdef JAEGERTRACING_WITH_YAML_CPP

    static Config parse(const YAML::Node& configYAML)
    {
        if (!configYAML.IsDefined() || !configYAML.IsMap()) {
            return Config();
        }

        const auto queueSize =
            utils::yaml::findOrDefault<int>(configYAML, "queueSize", 0);
        const auto bufferFlushInterval =
            std::chrono::seconds(utils::yaml::findOrDefault<int>(
                configYAML, "bufferFlushInterval", 0));
        const auto logSpans =
            utils::yaml::findOrDefault<bool>(configYAML, "logSpans", false);
        const auto localAgentHostPort = utils::yaml::findOrDefault<std::string>(
            configYAML, "localAgentHostPort", "");
        return Config(
            queueSize, bufferFlushInterval, logSpans, localAgentHostPort);
    }

#endif  // JAEGERTRACING_WITH_YAML_CPP

    explicit Config(
        int queueSize = kDefaultQueueSize,
        const Clock::duration& bufferFlushInterval =
            defaultBufferFlushInterval(),
        bool logSpans = false,
        const std::string& localAgentHostPort = kDefaultLocalAgentHostPort)
        : _queueSize(queueSize > 0 ? queueSize : kDefaultQueueSize)
        , _bufferFlushInterval(bufferFlushInterval.count() > 0
                                   ? bufferFlushInterval
                                   : defaultBufferFlushInterval())
        , _logSpans(logSpans)
        , _localAgentHostPort(localAgentHostPort.empty()
                                  ? kDefaultLocalAgentHostPort
                                  : localAgentHostPort)
    {
    }

    std::unique_ptr<Reporter> makeReporter(const std::string& serviceName,
                                           logging::Logger& logger,
                                           metrics::Metrics& metrics) const;

    int queueSize() const { return _queueSize; }

    const Clock::duration& bufferFlushInterval() const
    {
        return _bufferFlushInterval;
    }

    bool logSpans() const { return _logSpans; }

    const std::string& localAgentHostPort() const
    {
        return _localAgentHostPort;
    }

  private:
    int _queueSize;
    Clock::duration _bufferFlushInterval;
    bool _logSpans;
    std::string _localAgentHostPort;
};

}  // namespace reporters
}  // namespace jaegertracing

#endif  // JAEGERTRACING_REPORTERS_CONFIG_H
