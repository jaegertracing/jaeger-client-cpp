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

#include "jaegertracing/reporters/Config.h"
#include "jaegertracing/ThriftSender.h"
#include "jaegertracing/reporters/CompositeReporter.h"
#include "jaegertracing/reporters/LoggingReporter.h"
#include "jaegertracing/reporters/RemoteReporter.h"

namespace jaegertracing {
namespace reporters {

constexpr int Config::kDefaultQueueSize;
constexpr const char* Config::kDefaultLocalAgentHostPort;
constexpr const char* Config::kDefaultEndpoint;

std::unique_ptr<Reporter> Config::makeReporter(const std::string& serviceName,
                                               logging::Logger& logger,
                                               metrics::Metrics& metrics) const
{

    std::unique_ptr<utils::Transport> transporter =
        _endpoint.empty()
            ? (std::unique_ptr<utils::Transport>(new utils::UDPTransporter(
                  net::IPAddress::v4(_localAgentHostPort), 0)))
            : (std::unique_ptr<utils::Transport>(
                  new utils::HttpTransporter(net::URI::parse(_endpoint), 0)));

    std::unique_ptr<ThriftSender> sender(new ThriftSender(
        std::forward<std::unique_ptr<utils::Transport>>(transporter)));
    std::unique_ptr<RemoteReporter> remoteReporter(new RemoteReporter(
        _bufferFlushInterval, _queueSize, std::move(sender), logger, metrics));
    if (_logSpans) {
        logger.info("Initializing logging reporter");
        return std::unique_ptr<CompositeReporter>(new CompositeReporter(
            { std::shared_ptr<RemoteReporter>(std::move(remoteReporter)),
              std::make_shared<LoggingReporter>(logger) }));
    }
    return std::unique_ptr<Reporter>(std::move(remoteReporter));
}

}  // namespace reporters
}  // namespace jaegertracing
