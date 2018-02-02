# Build the jaegertracing library its self
cmake_minimum_required(VERSION 3.3)

set(SRC
    Config.cpp
    LogRecord.cpp
    Logging.cpp
    Reference.cpp
    Span.cpp
    SpanContext.cpp
    Tag.cpp
    TraceID.cpp
    Tracer.cpp
    Transport.cpp
    UDPTransport.cpp
    baggage/BaggageSetter.cpp
    baggage/RemoteRestrictionJSON.cpp
    baggage/RemoteRestrictionManager.cpp
    baggage/Restriction.cpp
    baggage/RestrictionManager.cpp
    baggage/RestrictionsConfig.cpp
    metrics/Counter.cpp
    metrics/Gauge.cpp
    metrics/InMemoryStatsReporter.cpp
    metrics/Metric.cpp
    metrics/Metrics.cpp
    metrics/NullCounter.cpp
    metrics/NullGauge.cpp
    metrics/NullStatsFactory.cpp
    metrics/NullStatsReporter.cpp
    metrics/NullTimer.cpp
    metrics/StatsFactory.cpp
    metrics/StatsFactoryImpl.cpp
    metrics/StatsReporter.cpp
    metrics/Timer.cpp
    net/IPAddress.cpp
    net/Socket.cpp
    net/URI.cpp
    net/http/Error.cpp
    net/http/Header.cpp
    net/http/Method.cpp
    net/http/Request.cpp
    net/http/Response.cpp
    platform/Endian.cpp
    platform/Hostname.cpp
    propagation/Extractor.cpp
    propagation/HeadersConfig.cpp
    propagation/Injector.cpp
    propagation/Propagator.cpp
    reporters/CompositeReporter.cpp
    reporters/Config.cpp
    reporters/InMemoryReporter.cpp
    reporters/LoggingReporter.cpp
    reporters/NullReporter.cpp
    reporters/RemoteReporter.cpp
    reporters/Reporter.cpp
    samplers/AdaptiveSampler.cpp
    samplers/Config.cpp
    samplers/ConstSampler.cpp
    samplers/GuaranteedThroughputProbabilisticSampler.cpp
    samplers/ProbabilisticSampler.cpp
    samplers/RateLimitingSampler.cpp
    samplers/RemoteSamplingJSON.cpp
    samplers/RemotelyControlledSampler.cpp
    samplers/Sampler.cpp
    samplers/SamplingStatus.cpp
    thrift-gen/Agent.cpp
    thrift-gen/AggregationValidator.cpp
    thrift-gen/BaggageRestrictionManager.cpp
    thrift-gen/Collector.cpp
    thrift-gen/Dependency.cpp
    thrift-gen/SamplingManager.cpp
    thrift-gen/TracedService.cpp
    thrift-gen/ZipkinCollector.cpp
    thrift-gen/agent_constants.cpp
    thrift-gen/agent_types.cpp
    thrift-gen/aggregation_validator_constants.cpp
    thrift-gen/aggregation_validator_types.cpp
    thrift-gen/baggage_constants.cpp
    thrift-gen/baggage_types.cpp
    thrift-gen/dependency_constants.cpp
    thrift-gen/dependency_types.cpp
    thrift-gen/jaeger_constants.cpp
    thrift-gen/jaeger_types.cpp
    thrift-gen/sampling_constants.cpp
    thrift-gen/sampling_types.cpp
    thrift-gen/tracetest_constants.cpp
    thrift-gen/tracetest_types.cpp
    thrift-gen/zipkincore_constants.cpp
    thrift-gen/zipkincore_types.cpp
    utils/ErrorUtil.cpp
    utils/HexParsing.cpp
    utils/RateLimiter.cpp
    utils/Regex.cpp
    utils/UDPClient.cpp
    utils/YAML.cpp)

add_library(jaegertracing SHARED ${SRC})
add_library(jaegertracing-static STATIC ${SRC})

if (JAEGERTRACING_COVERAGE)
  append_coverage_compiler_flags()
endif()

foreach(tgt jaegertracing jaegertracing-static)
  # This lets us use the "jaegertracing/" include prefix as if we were installed
  target_include_directories(${tgt} PRIVATE
    PRIVATE $<BUILD_INTERFACE:${jaegertracing_SOURCE_DIR}/src>
    PRIVATE $<BUILD_INTERFACE:${jaegertracing_BINARY_DIR}/src>)
  target_compile_features(${tgt} PUBLIC cxx_std_11)
  set_target_properties(${tgt} PROPERTIES
    VERSION ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR})
endforeach()

# Tell others where we'll install our headers for the cmake
# package output.
target_include_directories(jaegertracing-static INTERFACE
  PUBLIC $<INSTALL_INTERFACE:${include_install_dir}>)
target_include_directories(jaegertracing INTERFACE
  PUBLIC $<INSTALL_INTERFACE:${include_install_dir}>)

# This works whether Boost::regex is header-only or requires
# a linked library for older GCC
target_link_libraries(jaegertracing PUBLIC Boost::regex)
target_link_libraries(jaegertracing-static PUBLIC Boost::regex)

# Prefer shared Thrift for our own shared build, static Thrift
# for our own static build.
if (TARGET thrift::thrift)
  target_link_libraries(jaegertracing PUBLIC thrift::thrift)
else()
  target_link_libraries(jaegertracing PUBLIC thrift::thrift_static)
endif()
if (TARGET thrift::thrift_static)
  target_link_libraries(jaegertracing-static PUBLIC thrift::thrift_static)
else()
  target_link_libraries(jaegertracing-static PUBLIC thrift::thrift)
endif()

target_link_libraries(jaegertracing PUBLIC OpenTracing::opentracing)
target_link_libraries(jaegertracing-static PUBLIC OpenTracing::opentracing-static)

target_link_libraries(jaegertracing PUBLIC OpenTracing::opentracing)
target_link_libraries(jaegertracing-static PUBLIC OpenTracing::opentracing-static)

# This "link" will just add the header path via the INTERFACE library
target_link_libraries(jaegertracing PUBLIC nlohmann_json)
target_link_libraries(jaegertracing-static PUBLIC nlohmann_json)

if(JAEGERTRACING_WITH_YAML_CPP)
  # yaml-cpp doesn't usually provide a static lib, so link the shared lib
  target_link_libraries(jaegertracing PUBLIC yaml-cpp::yaml-cpp)
  target_link_libraries(jaegertracing-static PUBLIC yaml-cpp::yaml-cpp)
endif()

configure_file(
  Constants.h.in
  Constants.h
  @ONLY)
set(CMAKE_INSTALL_CURRENT_DIR ON)

include(CMakeLists-UnitTest.cmake)

# Installation
#
# Targets:
#   * <prefix>/lib/libjaegertracing.a
#   * <prefix>/lib/libjaegertracing.so
#   * header location after install: <prefix>/include/jaegertracing/Tracer.h
#   * headers can be included by C++ code `#include <jaegertracing/Tracer.h>`
install(
  TARGETS jaegertracing jaegertracing-static
  EXPORT "${TARGETS_EXPORT_NAME}"
  LIBRARY DESTINATION "lib"
  ARCHIVE DESTINATION "lib"
  RUNTIME DESTINATION "bin"
  INCLUDES DESTINATION "${include_install_dir}"
)

# Headers:
#   * Tracer.h -> <prefix>/include/jaegertracing/Tracer.h
install(DIRECTORY "${jaegertracing_SOURCE_DIR}/src/jaegertracing"
        DESTINATION "${include_install_dir}"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "testutils/*.h" EXCLUDE)

#   * build/src/jaegertracing/Constants.h ->
#     <prefix>/include/jaegertracing/Constants.h
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/Constants.h"
        DESTINATION "${include_install_dir}/jaegertracing")
