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

#ifndef JAEGERTRACING_PLATFORM_ENDIAN_H
#define JAEGERTRACING_PLATFORM_ENDIAN_H

#include <cstdint>

#if defined(__APPLE__)

#include <libkern/OSByteOrder.h>
#define htobe16(x) OSSwapHostToBigInt16(x)
#define htobe32(x) OSSwapHostToBigInt32(x)
#define htobe64(x) OSSwapHostToBigInt64(x)
#define be16toh(x) OSSwapBigToHostInt16(x)
#define be32toh(x) OSSwapBigToHostInt32(x)
#define be64toh(x) OSSwapBigToHostInt64(x)

#elif defined(__linux__)
#include <endian.h>
#else
#error "unsupported platform"
#endif

namespace jaegertracing {
namespace platform {
namespace endian {

inline uint16_t toBigEndian(uint16_t value) { return htobe16(value); }

inline uint32_t toBigEndian(uint32_t value) { return htobe32(value); }

inline uint64_t toBigEndian(uint64_t value) { return htobe64(value); }

inline uint16_t fromBigEndian(uint16_t value) { return be16toh(value); }

inline uint32_t fromBigEndian(uint32_t value) { return be32toh(value); }

inline uint64_t fromBigEndian(uint64_t value) { return be64toh(value); }

}  // namespace endian
}  // namespace platform
}  // namespace jaegertracing

#endif  // JAEGERTRACING_PLATFORM_ENDIAN_H
