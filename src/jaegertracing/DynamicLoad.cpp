/*
 * Copyright (c) 2018 Uber Technologies, Inc.
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
#include <cassert>
#include <cstring>
#include <system_error>

#include <opentracing/dynamic_load.h>

#include "jaegertracing/DynamicLoad.h"
#include "jaegertracing/Tracer.h"
#include "jaegertracing/TracerFactory.h"

int JaegerTracingMakeTracerFactoryFct(const char* opentracing_version,
                                           const char* opentracing_abi_version,
                                           const void** error_category,
                                           void* error_message,
                                           void** tracer_factory)
{
    assert(error_category != nullptr);
    assert(tracer_factory != nullptr);
#ifndef JAEGERTRACING_WITH_YAML_CPP
    *errorCategory =
        static_cast<const void*>(&opentracing::dynamic_load_error_category());
    return opentracing::dynamic_load_not_supported_error.value();
#endif
    if (std::strcmp(opentracing_version, OPENTRACING_VERSION) != 0 ||
        std::strcmp(opentracing_abi_version, OPENTRACING_ABI_VERSION) != 0) {
        *error_category = static_cast<const void*>(
            &opentracing::dynamic_load_error_category());
        return opentracing::incompatible_library_versions_error.value();
    }

    *tracer_factory = new (std::nothrow) jaegertracing::TracerFactory{};
    if (*tracer_factory == nullptr) {
        *error_category = static_cast<const void*>(&std::generic_category());
        return static_cast<int>(std::errc::not_enough_memory);
    }

    return 0;
}

OPENTRACING_DECLARE_IMPL_FACTORY(JaegerTracingMakeTracerFactoryFct)
