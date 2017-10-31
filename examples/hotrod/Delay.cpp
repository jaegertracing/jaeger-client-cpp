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

#include "Delay.h"

#include <thread>

namespace jaegertracing {
namespace examples {
namespace hotrod {
namespace delay {
void sleep(const Clock::duration& average,
           const Clock::duration& standardDeviation)
{
    static std::random_device device;
    static std::mt19937 gen(device());
    static std::mutex mutex;

    using FractionalSecond = std::chrono::duration<double>;
    std::normal_distribution<> distribution(
        std::chrono::duration_cast<FractionalSecond>(average).count(),
        std::chrono::duration_cast<FractionalSecond>(
            standardDeviation).count());
    FractionalSecond secondsToSleep;
    {
        std::lock_guard<std::mutex> lock(mutex);
        secondsToSleep = FractionalSecond(distribution(gen));
    }
    std::this_thread::sleep_for(secondsToSleep);
}

}  // namespace delay
}  // namespace hotrod
}  // namespace examples
}  // namespace jaegertracing
