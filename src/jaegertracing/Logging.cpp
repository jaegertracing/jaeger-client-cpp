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

#include "jaegertracing/Logging.h"

#include <mutex>

#include <spdlog/sinks/null_sink.h>

namespace jaegertracing {
namespace logging {

std::shared_ptr<spdlog::logger> nullLogger()
{
    static auto logger =
        spdlog::create("null", std::make_shared<spdlog::sinks::null_sink_mt>());
    return logger;
}

std::shared_ptr<spdlog::logger> consoleLogger()
{
    static auto logger = spdlog::stdout_color_mt("console");
    return logger;
}

}  // namespace logging
}  // namespace jaegertracing
