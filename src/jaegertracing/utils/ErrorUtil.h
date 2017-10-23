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

#ifndef JAEGERTRACING_UTILS_ERRORUTIL_H
#define JAEGERTRACING_UTILS_ERRORUTIL_H

#include <string>
#include <system_error>

#include "jaegertracing/Logging.h"
#include "jaegertracing/Transport.h"

namespace jaegertracing {
namespace utils {
namespace ErrorUtil {

inline void logError(spdlog::logger& logger,
                     const std::string& message,
                     spdlog::level::level_enum level = spdlog::level::err)
{
    try {
        throw;
    } catch (const Transport::Exception& ex) {
        logger.log(level,
                   "{0}: {1}, numFailed={2}",
                   message,
                   ex.what(),
                   ex.numFailed());
    } catch (const std::system_error& ex) {
        logger.log(
            level, "{0}: {1}, code={2}", message, ex.what(), ex.code().value());
    } catch (const std::exception& ex) {
        logger.log(level, "{0}: {1}", message, ex.what());
    } catch (...) {
        logger.log(level, message);
    }
}

}  // namespace ErrorUtil
}  // namespace utils
}  // namespace jaegertracing

#endif  // JAEGERTRACING_UTILS_ERRORUTIL_H
