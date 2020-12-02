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

#ifndef JAEGERTRACING_PROPAGATION_HEADERSCONFIG_H
#define JAEGERTRACING_PROPAGATION_HEADERSCONFIG_H

#include "jaegertracing/Constants.h"
#include "jaegertracing/propagation/Format.h"
#include "jaegertracing/utils/EnvVariable.h"
#include "jaegertracing/utils/YAML.h"
#include <string>

namespace jaegertracing {
namespace propagation {

class HeadersConfig {
  public:

    static constexpr auto kJAEGER_PROPAGATION_ENV_PROP = "JAEGER_PROPAGATION";

#ifdef JAEGERTRACING_WITH_YAML_CPP

    static HeadersConfig parse(const YAML::Node& configYAML)
    {
        if (!configYAML.IsDefined() || !configYAML.IsMap()) {
            return HeadersConfig();
        }

        const auto jaegerDebugHeader = utils::yaml::findOrDefault<std::string>(
            configYAML, "jaegerDebugHeader", "");
        const auto jaegerBaggageHeader =
            utils::yaml::findOrDefault<std::string>(
                configYAML, "jaegerBaggageHeader", "");
        const auto traceContextHeaderName =
            utils::yaml::findOrDefault<std::string>(
                configYAML, "TraceContextHeaderName", "");
        const auto traceBaggageHeaderPrefix =
            utils::yaml::findOrDefault<std::string>(
                configYAML, "traceBaggageHeaderPrefix", "");
        const auto traceContextHeaderFormat =
            utils::yaml::findOrDefault<std::string>(
                configYAML, "TraceContextHeaderFormat", "");

        return HeadersConfig(
            jaegerDebugHeader,
            jaegerBaggageHeader,
            traceContextHeaderName,
            traceBaggageHeaderPrefix,
            traceContextHeaderFormat == "w3c" ? Format::W3C : Format::JAEGER);
    }

#endif  // JAEGERTRACING_WITH_YAML_CPP

    HeadersConfig()
        : HeadersConfig("", "", "", "")
    {
    }

    HeadersConfig(const std::string& jaegerDebugHeader,
                  const std::string& jaegerBaggageHeader,
                  const std::string& traceContextHeaderName,
                  const std::string& traceBaggageHeaderPrefix)
        : _jaegerDebugHeader(jaegerDebugHeader.empty() ? kJaegerDebugHeader
                                                       : jaegerDebugHeader)
        , _jaegerBaggageHeader(jaegerBaggageHeader.empty()
                                   ? kJaegerBaggageHeader
                                   : jaegerBaggageHeader)
        , _traceContextHeaderName(traceContextHeaderName.empty()
                                      ? kTraceContextHeaderName
                                      : traceContextHeaderName)
        , _traceBaggageHeaderPrefix(traceBaggageHeaderPrefix.empty()
                                        ? kTraceBaggageHeaderPrefix
                                        : traceBaggageHeaderPrefix)
        , _traceContextHeaderFormat(Format::JAEGER)
    {
    }

    HeadersConfig(const std::string& jaegerDebugHeader,
                  const std::string& jaegerBaggageHeader,
                  const std::string& traceContextHeaderName,
                  const std::string& traceBaggageHeaderPrefix,
                  Format traceContextHeaderFormat)
        : _jaegerDebugHeader(jaegerDebugHeader.empty() ? kJaegerDebugHeader
                                                       : jaegerDebugHeader)
        , _jaegerBaggageHeader(jaegerBaggageHeader.empty()
                                   ? kJaegerBaggageHeader
                                   : jaegerBaggageHeader)
        , _traceContextHeaderName(traceContextHeaderName.empty()
                                      ? (traceContextHeaderFormat == Format::W3C
                                             ? kW3CTraceContextHeaderName
                                             : kTraceContextHeaderName)
                                      : traceContextHeaderName)
        , _traceBaggageHeaderPrefix(traceBaggageHeaderPrefix.empty()
                                        ? kTraceBaggageHeaderPrefix
                                        : traceBaggageHeaderPrefix)
        , _traceContextHeaderFormat(traceContextHeaderFormat)
    {
    }

    const std::string& jaegerBaggageHeader() const
    {
        return _jaegerBaggageHeader;
    }

    const std::string& jaegerDebugHeader() const { return _jaegerDebugHeader; }

    const std::string& traceBaggageHeaderPrefix() const
    {
        return _traceBaggageHeaderPrefix;
    }

    const std::string& traceContextHeaderName() const
    {
        return _traceContextHeaderName;
    }

    Format traceContextHeaderFormat() const
    {
        return _traceContextHeaderFormat;
    }

    void fromEnv() {
        const auto propagationFormat =
        utils::EnvVariable::getStringVariable(kJAEGER_PROPAGATION_ENV_PROP);
        if (!propagationFormat.empty()) {
            if(propagationFormat == "jaeger") {
                _traceContextHeaderName = kTraceContextHeaderName;
                _traceContextHeaderFormat = Format::JAEGER;
            } else if(propagationFormat == "w3c") {
                _traceContextHeaderName = kW3CTraceContextHeaderName;
                _traceContextHeaderFormat = Format::W3C;
            }
        }
    }

  private:
    std::string _jaegerDebugHeader;
    std::string _jaegerBaggageHeader;
    std::string _traceContextHeaderName;
    std::string _traceBaggageHeaderPrefix;
    Format _traceContextHeaderFormat;
};

}  // namespace propagation
}  // namespace jaegertracing

#endif  // JAEGERTRACING_PROPAGATION_HEADERSCONFIG_H
