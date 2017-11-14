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

#ifndef JAEGERTRACING_SAMPLERS_PROBABILISTICSAMPLER_H
#define JAEGERTRACING_SAMPLERS_PROBABILISTICSAMPLER_H

#include "jaegertracing/Constants.h"
#include "jaegertracing/samplers/Sampler.h"

namespace jaegertracing {
namespace samplers {

class ProbabilisticSampler : public Sampler {
  public:
    explicit ProbabilisticSampler(double samplingRate)
        : _samplingRate(std::max(0.0, std::min(samplingRate, 1.0)))
        , _samplingBoundary(
              static_cast<uint64_t>(
                std::llround(kMaxRandomNumber * _samplingRate)))
        , _tags({ { kSamplerTypeTagKey, kSamplerTypeProbabilistic },
                  { kSamplerParamTagKey, _samplingRate } })
    {
    }

    double samplingRate() const { return _samplingRate; }

    SamplingStatus isSampled(const TraceID& id,
                             const std::string& operation) override
    {
        return SamplingStatus(_samplingBoundary >= id.low(), _tags);
    }

    void close() override {}

    Type type() const override { return Type::kProbabilisticSampler; }

  private:
    static constexpr auto kMaxRandomNumber =
        std::numeric_limits<uint64_t>::max();

    double _samplingRate;
    uint64_t _samplingBoundary;
    std::vector<Tag> _tags;
};

}  // namespace samplers
}  // namespace jaegertracing

#endif  // JAEGERTRACING_SAMPLERS_PROBABILISTICSAMPLER_H
