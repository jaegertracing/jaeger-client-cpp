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

#ifndef JAEGERTRACING_PLATFORM_UDP_H
#define JAEGERTRACING_PLATFORM_UDP_H

#include <unistd.h>

#include <string>
#include <system_error>

namespace jaegertracing {
namespace platform {

#if __APPLE__
static constexpr auto kUDPPacketMaxLength = 9200;
#else
static constexpr auto kUDPPacketMaxLength = 65000;
#endif  // __APPLE__

}  // namespace platform
}  // namespace jaegertracing

#endif  // JAEGERTRACING_PLATFORM_UDP_H
