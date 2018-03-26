/*
 * Copyright (c) 2017-2018 Uber Technologies, Inc.
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

#ifndef JAEGERTRACING_UTILS_REGEX_H
#define JAEGERTRACING_UTILS_REGEX_H

#if (!defined(__GNUC__) || defined(__clang__) || __GNUC__ > 4 ||               \
     __GNUC_MINOR__ > 8)
#define JAEGERTRACING_USE_STD_REGEX 1
#else
#define JAEGERTRACING_USE_STD_REGEX 0
#endif

#if JAEGERTRACING_USE_STD_REGEX
#include <regex>
#else
#include <boost/regex.hpp>
#endif

#if JAEGERTRACING_USE_STD_REGEX
namespace regex_namespace = std;
#else
namespace regex_namespace = boost;
#endif

#endif  // JAEGERTRACING_UTILS_REGEX_H
