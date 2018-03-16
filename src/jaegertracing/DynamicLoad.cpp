#include <cstring>
#include <system_error>

#include <opentracing/dynamic_load.h>

#include "TracerFactory.h"
#include "jaegertracing/Tracer.h"

int OpenTracingMakeTracerFactory(const char* opentracingVersion,
                                 const void** errorCategory,
                                 void** tracerFactory)
{
    if (std::strcmp(opentracingVersion, OPENTRACING_VERSION) != 0) {
        *errorCategory = static_cast<const void*>(
            &opentracing::dynamic_load_error_category());
        return opentracing::incompatible_library_versions_error.value();
    }

    *tracerFactory = new (std::nothrow) jaegertracing::TracerFactory{};
    if (*tracerFactory == nullptr) {
        *errorCategory = static_cast<const void*>(&std::generic_category());
        return static_cast<int>(std::errc::not_enough_memory);
    }

    return 0;
}
