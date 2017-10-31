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

#ifndef JAEGERTRACING_EXAMPLES_HOTROD_DELAY_H
#define JAEGERTRACING_EXAMPLES_HOTROD_DELAY_H

#include <chrono>
#include <mutex>
#include <random>

namespace jaegertracing {
namespace examples {
namespace hotrod {
namespace delay {

using Clock = std::chrono::steady_clock;

void sleep(const Clock::duration& average,
           const Clock::duration& standardDeviation);

}  // namespace delay
}  // namespace hotrod
}  // namespace examples
}  // namespace jaegertracing

#endif  // JAEGERTRACING_EXAMPLES_HOTROD_DELAY_H
