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
#include "jaegertracing/utils/YAML.h"
#include <iostream>
#include <string>
namespace jaegertracing {
namespace propagation {

class HeadersConfig {
  public:
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

        const auto enableZipkinHeaders =
                utils::yaml::findOrDefault<bool>(
                        configYAML, "enableZipkinHeaders", false);
        const auto zipkinTraceIdHeaderName =             utils::yaml::findOrDefault<std::string>(
                configYAML, "zipkinTraceIdHeaderName", "");
        const auto zipkinSpanIdHeaderName =             utils::yaml::findOrDefault<std::string>(
                configYAML, "zipkinSpanIdHeaderName", "");
        const auto ZipkinParentSpanIdHederName = utils::yaml::findOrDefault<std::string>(
                configYAML, "zipkinParentSpanIdHederName", "");
        const auto zipkinSampledHeaderName =             utils::yaml::findOrDefault<std::string>(
                configYAML, "zipkinSampledHeaderName", "");

        return HeadersConfig(jaegerDebugHeader,
                             jaegerBaggageHeader,
                             traceContextHeaderName,
                             traceBaggageHeaderPrefix,
                             enableZipkinHeaders,
                             zipkinTraceIdHeaderName,
                             ZipkinParentSpanIdHederName,
                             zipkinSpanIdHeaderName,
                             zipkinSampledHeaderName);
    }

#endif  // JAEGERTRACING_WITH_YAML_CPP

    HeadersConfig()
        : HeadersConfig("", "", "", "", false,"", "", "", "")
    {
    }
    HeadersConfig(bool enableZipkinHeaders)
            : HeadersConfig("", "", "", "", enableZipkinHeaders,"", "", "", "")

    {

    }

    HeadersConfig(const std::string& jaegerDebugHeader,
                  const std::string& jaegerBaggageHeader,
                  const std::string& traceContextHeaderName,
                  const std::string& traceBaggageHeaderPrefix,
                  const bool enableZipkinHeaders,
                  const std::string& zipkinTraceIdHeaderName,
                  const std::string& zipkinParentSpanIdHeaderName,
                  const std::string& zipkinSpanIdHeaderName,
                  const std::string& zipkinTraceSampledHeaderName
                  )
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
        ,_zipkinTraceIdHeaderName(zipkinTraceIdHeaderName.empty()
                                  ?kzipkinTraceIdHeaderName:zipkinTraceIdHeaderName)
        ,_zipkinParentSpanIdHeaderName(zipkinParentSpanIdHeaderName.empty()
                                  ?kzipkinParentSpanIdHeaderName:zipkinParentSpanIdHeaderName)
        ,_zipkinSpanIdHeaderName(zipkinSpanIdHeaderName.empty()
                                 ?kzipkinSpanIdHeaderName:zipkinSpanIdHeaderName)
        ,_zipkinTraceSampledHeaderName(zipkinTraceSampledHeaderName.empty()
                                  ?kzipkinTraceSampledHeaderName:zipkinTraceSampledHeaderName)
        ,_enableZipkinHeaders(enableZipkinHeaders)
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
    const std::string& zipkinTraceIdHeaderName() const
    {
        return _zipkinTraceIdHeaderName;
    }
    const std::string& zipkinSpanIdHeaderName() const
    {
        return _zipkinSpanIdHeaderName;
    }
    const std::string& zipkinParentSPanIdHeaderName() const
    {
        return _zipkinParentSpanIdHeaderName;
    }
    const std::string& zipkinTraceSampledHeaderName() const
    {
        return _zipkinTraceSampledHeaderName;
    }
    const bool enableZipkinHeaders() const{
        return _enableZipkinHeaders;
    }
  private:
    std::string _jaegerDebugHeader;
    std::string _jaegerBaggageHeader;
    std::string _traceContextHeaderName;
    std::string _traceBaggageHeaderPrefix;

    std::string _zipkinTraceIdHeaderName;
    std::string _zipkinParentSpanIdHeaderName;
    std::string _zipkinSpanIdHeaderName;
    std::string _zipkinTraceSampledHeaderName;
    bool _enableZipkinHeaders;
    };

}  // namespace propagation
}  // namespace jaegertracing

#endif  // JAEGERTRACING_PROPAGATION_HEADERSCONFIG_H
