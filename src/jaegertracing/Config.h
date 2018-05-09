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

#ifndef JAEGERTRACING_CONFIG_H
#define JAEGERTRACING_CONFIG_H

#include "jaegertracing/Constants.h"
#include "jaegertracing/baggage/RestrictionsConfig.h"
#include "jaegertracing/propagation/HeadersConfig.h"
#include "jaegertracing/reporters/Config.h"
#include "jaegertracing/samplers/Config.h"
#include "jaegertracing/utils/YAML.h"

namespace jaegertracing {

class Config {
  public:
#ifdef JAEGERTRACING_WITH_YAML_CPP

    static Config parse(const YAML::Node& configYAML)
    {
        if (!configYAML.IsDefined() || !configYAML.IsMap()) {
            return Config();
        }

        const auto disabled =
            utils::yaml::findOrDefault<bool>(configYAML, "disabled", false);
        const auto samplerNode = configYAML["sampler"];
        const auto sampler = samplers::Config::parse(samplerNode);
        const auto reporterNode = configYAML["reporter"];
        const auto reporter = reporters::Config::parse(reporterNode);
        const auto headersNode = configYAML["headers"];
        const auto headers = propagation::HeadersConfig::parse(headersNode);
        const auto baggageRestrictionsNode = configYAML["baggage_restrictions"];
        const auto baggageRestrictions =
            baggage::RestrictionsConfig::parse(baggageRestrictionsNode);
        return Config(
            disabled, sampler, reporter, headers, baggageRestrictions);
    }

#endif  // JAEGERTRACING_WITH_YAML_CPP

    explicit Config(bool disabled = false,
                    const samplers::Config& sampler = samplers::Config(),
                    const reporters::Config& reporter = reporters::Config(),
                    const propagation::HeadersConfig& headers =
                        propagation::HeadersConfig(),
                    const baggage::RestrictionsConfig& baggageRestrictions =
                        baggage::RestrictionsConfig())
        : _disabled(disabled)
        , _sampler(sampler)
        , _reporter(reporter)
        , _headers(headers)
        , _baggageRestrictions(baggageRestrictions)
    {
    }

    bool disabled() const { return _disabled; }

    const samplers::Config& sampler() const { return _sampler; }

    const reporters::Config& reporter() const { return _reporter; }

    const propagation::HeadersConfig& headers() const { return _headers; }

    const baggage::RestrictionsConfig& baggageRestrictions() const
    {
        return _baggageRestrictions;
    }

  private:
    bool _disabled;
    samplers::Config _sampler;
    reporters::Config _reporter;
    propagation::HeadersConfig _headers;
    baggage::RestrictionsConfig _baggageRestrictions;
};

}  // namespace jaegertracing

#endif  // JAEGERTRACING_CONFIG_H
