cmake_minimum_required(VERSION 3.3)

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_SOURCE_DIR}/cmake/toolchain.cmake")

set(HUNTER_CACHE_SERVERS "https://github.com/jaegertracing/cpp-client-hunter-cache")

include(CMakeDependentOption)
include(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.174.tar.gz"
    SHA1 "712898528fb6dc8532454e2e323404d93a76d8ec"
    LOCAL
)

# Copied from [cpp-ethereum]
# (https://github.com/ethereum/cpp-ethereum/blob/develop/CMakeLists.txt)
if(HUNTER_ENABLED)
  find_package(PythonInterp QUIET)
  if(PYTHONINTERP_FOUND)
    hunter_gate_self(
        "${HUNTER_CACHED_ROOT}"
        "${HUNTER_VERSION}"
        "${HUNTER_SHA1}"
        hunter_dir
    )
    set(hunter_upload_script
        "${hunter_dir}/maintenance/upload-cache-to-github.py")
    set(hunter_cache_dir "${HUNTER_GATE_ROOT}/_Base/Cache")
    set(hunter_tmp_dir "${HUNTER_GATE_ROOT}/tmp")
    add_custom_target(
        hunter_upload_cache
        ${PYTHON_EXECUTABLE} ${hunter_upload_script}
        --username jaegertracing
        --repo-owner jaegertracing
        --repo cpp-client-hunter-cache
        --cache-dir ${hunter_cache_dir}
        --temp-dir ${hunter_tmp_dir})
  endif()
endif()

project(jaegertracing VERSION 0.2.0)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" OR
    CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Werror -pedantic")
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-private-field")
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS true)

if(HUNTER_ENABLED)
  set(hunter_config "CONFIG")
else()
  set(hunter_config "")
endif()

set(package_deps)

if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND
    CMAKE_CXX_COMPILER_VERSION VERSION_LESS "4.9")
  set(boost_components regex)
  hunter_add_package(Boost COMPONENTS regex)
  list(APPEND LIBS Boost::regex)
else()
  set(boost_components)
  hunter_add_package(Boost)
endif()
find_package(Boost ${hunter_config} REQUIRED ${boost_components})
list(APPEND package_deps Boost)

hunter_add_package(thrift)
find_package(thrift ${hunter_config} REQUIRED)
if(HUNTER_ENABLED)
  list(APPEND LIBS thrift::thrift_static)
else()
  list(APPEND LIBS ${THRIFT_LIBRARIES})
endif()
list(APPEND package_deps thrift)

hunter_add_package(opentracing-cpp)
# Not `${hunter_config}` because OpenTracing provides its own
# OpenTracingConfig.cmake file
find_package(OpenTracing CONFIG REQUIRED)
list(APPEND LIBS OpenTracing::opentracing-static)
list(APPEND package_deps opentracing-cpp)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json CONFIG REQUIRED)
list(APPEND LIBS nlohmann_json)
list(APPEND package_deps nlohmann_json)

option(JAEGERTRACING_COVERAGE "Build with coverage" $ENV{COVERAGE})
option(JAEGERTRACING_BUILD_CROSSDOCK "Build crossdock" $ENV{CROSSDOCK})
cmake_dependent_option(
  JAEGERTRACING_WITH_YAML_CPP "Use yaml-cpp to parse config files" ON
  "NOT JAEGERTRACING_BUILD_CROSSDOCK" ON)

if(JAEGERTRACING_WITH_YAML_CPP)
  hunter_add_package(yaml-cpp)
  # Not `${hunter_config}` because yaml-cpp provides its own
  # yaml-cpp-config.cmake file
  find_package(yaml-cpp CONFIG REQUIRED)
  if(HUNTER_ENABLED)
      list(APPEND LIBS yaml-cpp::yaml-cpp)
  else()
      list(APPEND LIBS yaml-cpp)
  endif()
  list(APPEND package_deps yaml-cpp)
endif()

include(CTest)
if(BUILD_TESTING)
  hunter_add_package(GTest)
  find_package(GTest ${hunter_config} REQUIRED)

  if(JAEGERTRACING_COVERAGE)
      include(CodeCoverage)
      append_coverage_compiler_flags()
      set(COVERAGE_EXCLUDES "${CMAKE_SOURCE_DIR}/src/jaegertracing/thrift-gen/*"
                            "*Test.cpp")
  endif()
endif()

if(JAEGERTRACING_BUILD_CROSSDOCK)
  set(CROSSDOCK_SRC crossdock/Server.cpp)
  add_executable(crossdock ${CROSSDOCK_SRC})
  target_include_directories(crossdock PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/crossdock>)
  target_link_libraries(crossdock PUBLIC jaegertracing-static)

  set(JAEGER_CROSSDOCK_URL
"https://raw.githubusercontent.com/jaegertracing/jaeger/master/docker-compose/\
jaeger-docker-compose.yml")
  file(DOWNLOAD ${JAEGER_CROSSDOCK_URL}
       "${PROJECT_SOURCE_DIR}/crossdock/jaeger-docker-compose.yml")
  find_program(DOCKER_COMPOSE_EXE docker-compose REQUIRED)
  set(DOCKER_COMPOSE_CMD ${DOCKER_COMPOSE_EXE}
      -f ${PROJECT_SOURCE_DIR}/crossdock/docker-compose.yml
      -f ${PROJECT_SOURCE_DIR}/crossdock/jaeger-docker-compose.yml)
  add_custom_target(crossdock-kill
    COMMAND ${DOCKER_COMPOSE_CMD} kill
    COMMAND ${DOCKER_COMPOSE_CMD} rm --force)
  add_custom_target(crossdock-run
    COMMAND ${DOCKER_COMPOSE_CMD} build
    COMMAND ${DOCKER_COMPOSE_CMD} run crossdock
    DEPENDS crossdock-kill)
  add_custom_target(crossdock-fresh
    COMMAND ${DOCKER_COMPOSE_CMD} pull
    COMMAND ${DOCKER_COMPOSE_CMD} build
    COMMAND ${DOCKER_COMPOSE_CMD} run crossdock
    DEPENDS crossdock-kill)
  add_custom_target(crossdock-logs
    COMMAND ${DOCKER_COMPOSE_CMD} logs)
endif()

set(SRC
    src/jaegertracing/Config.cpp
    src/jaegertracing/LogRecord.cpp
    src/jaegertracing/Logging.cpp
    src/jaegertracing/Reference.cpp
    src/jaegertracing/Span.cpp
    src/jaegertracing/SpanContext.cpp
    src/jaegertracing/Tag.cpp
    src/jaegertracing/TraceID.cpp
    src/jaegertracing/Tracer.cpp
    src/jaegertracing/Transport.cpp
    src/jaegertracing/UDPTransport.cpp
    src/jaegertracing/baggage/BaggageSetter.cpp
    src/jaegertracing/baggage/RemoteRestrictionJSON.cpp
    src/jaegertracing/baggage/RemoteRestrictionManager.cpp
    src/jaegertracing/baggage/Restriction.cpp
    src/jaegertracing/baggage/RestrictionManager.cpp
    src/jaegertracing/baggage/RestrictionsConfig.cpp
    src/jaegertracing/metrics/Counter.cpp
    src/jaegertracing/metrics/Gauge.cpp
    src/jaegertracing/metrics/InMemoryStatsReporter.cpp
    src/jaegertracing/metrics/Metric.cpp
    src/jaegertracing/metrics/Metrics.cpp
    src/jaegertracing/metrics/NullCounter.cpp
    src/jaegertracing/metrics/NullGauge.cpp
    src/jaegertracing/metrics/NullStatsFactory.cpp
    src/jaegertracing/metrics/NullStatsReporter.cpp
    src/jaegertracing/metrics/NullTimer.cpp
    src/jaegertracing/metrics/StatsFactory.cpp
    src/jaegertracing/metrics/StatsFactoryImpl.cpp
    src/jaegertracing/metrics/StatsReporter.cpp
    src/jaegertracing/metrics/Timer.cpp
    src/jaegertracing/net/IPAddress.cpp
    src/jaegertracing/net/Socket.cpp
    src/jaegertracing/net/URI.cpp
    src/jaegertracing/net/http/Error.cpp
    src/jaegertracing/net/http/Header.cpp
    src/jaegertracing/net/http/Method.cpp
    src/jaegertracing/net/http/Request.cpp
    src/jaegertracing/net/http/Response.cpp
    src/jaegertracing/platform/Endian.cpp
    src/jaegertracing/platform/Hostname.cpp
    src/jaegertracing/propagation/Extractor.cpp
    src/jaegertracing/propagation/HeadersConfig.cpp
    src/jaegertracing/propagation/Injector.cpp
    src/jaegertracing/propagation/Propagator.cpp
    src/jaegertracing/reporters/CompositeReporter.cpp
    src/jaegertracing/reporters/Config.cpp
    src/jaegertracing/reporters/InMemoryReporter.cpp
    src/jaegertracing/reporters/LoggingReporter.cpp
    src/jaegertracing/reporters/NullReporter.cpp
    src/jaegertracing/reporters/RemoteReporter.cpp
    src/jaegertracing/reporters/Reporter.cpp
    src/jaegertracing/samplers/AdaptiveSampler.cpp
    src/jaegertracing/samplers/Config.cpp
    src/jaegertracing/samplers/ConstSampler.cpp
    src/jaegertracing/samplers/GuaranteedThroughputProbabilisticSampler.cpp
    src/jaegertracing/samplers/ProbabilisticSampler.cpp
    src/jaegertracing/samplers/RateLimitingSampler.cpp
    src/jaegertracing/samplers/RemoteSamplingJSON.cpp
    src/jaegertracing/samplers/RemotelyControlledSampler.cpp
    src/jaegertracing/samplers/Sampler.cpp
    src/jaegertracing/samplers/SamplingStatus.cpp
    src/jaegertracing/thrift-gen/Agent.cpp
    src/jaegertracing/thrift-gen/AggregationValidator.cpp
    src/jaegertracing/thrift-gen/BaggageRestrictionManager.cpp
    src/jaegertracing/thrift-gen/Collector.cpp
    src/jaegertracing/thrift-gen/Dependency.cpp
    src/jaegertracing/thrift-gen/SamplingManager.cpp
    src/jaegertracing/thrift-gen/TracedService.cpp
    src/jaegertracing/thrift-gen/ZipkinCollector.cpp
    src/jaegertracing/thrift-gen/agent_constants.cpp
    src/jaegertracing/thrift-gen/agent_types.cpp
    src/jaegertracing/thrift-gen/aggregation_validator_constants.cpp
    src/jaegertracing/thrift-gen/aggregation_validator_types.cpp
    src/jaegertracing/thrift-gen/baggage_constants.cpp
    src/jaegertracing/thrift-gen/baggage_types.cpp
    src/jaegertracing/thrift-gen/dependency_constants.cpp
    src/jaegertracing/thrift-gen/dependency_types.cpp
    src/jaegertracing/thrift-gen/jaeger_constants.cpp
    src/jaegertracing/thrift-gen/jaeger_types.cpp
    src/jaegertracing/thrift-gen/sampling_constants.cpp
    src/jaegertracing/thrift-gen/sampling_types.cpp
    src/jaegertracing/thrift-gen/tracetest_constants.cpp
    src/jaegertracing/thrift-gen/tracetest_types.cpp
    src/jaegertracing/thrift-gen/zipkincore_constants.cpp
    src/jaegertracing/thrift-gen/zipkincore_types.cpp
    src/jaegertracing/utils/ErrorUtil.cpp
    src/jaegertracing/utils/HexParsing.cpp
    src/jaegertracing/utils/RateLimiter.cpp
    src/jaegertracing/utils/Regex.cpp
    src/jaegertracing/utils/UDPClient.cpp
    src/jaegertracing/utils/YAML.cpp)

add_library(jaegertracing SHARED ${SRC})
target_include_directories(jaegertracing PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>)
target_link_libraries(jaegertracing ${LIBS})
set_target_properties(jaegertracing PROPERTIES
  VERSION ${PROJECT_VERSION}
  SOVERSION ${PROJECT_VERSION_MAJOR})

add_library(jaegertracing-static STATIC ${SRC})
target_include_directories(jaegertracing-static PUBLIC
  $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${PROJECT_BINARY_DIR}/src>)
target_link_libraries(jaegertracing-static ${LIBS})
set_target_properties(jaegertracing-static PROPERTIES
  OUTPUT_NAME jaegertracing)

configure_file(
  src/jaegertracing/Constants.h.in
  src/jaegertracing/Constants.h
  @ONLY)
set(CMAKE_INSTALL_CURRENT_DIR ON)

if(BUILD_TESTING)
  add_subdirectory("${CMAKE_SOURCE_DIR}/src/jaegertracing/testutils")
  set(TEST_SRC
      src/jaegertracing/ConfigTest.cpp
      src/jaegertracing/ReferenceTest.cpp
      src/jaegertracing/SpanContextTest.cpp
      src/jaegertracing/SpanTest.cpp
      src/jaegertracing/TagTest.cpp
      src/jaegertracing/TraceIDTest.cpp
      src/jaegertracing/TracerTest.cpp
      src/jaegertracing/UDPTransportTest.cpp
      src/jaegertracing/baggage/BaggageTest.cpp
      src/jaegertracing/metrics/MetricsTest.cpp
      src/jaegertracing/metrics/NullStatsFactoryTest.cpp
      src/jaegertracing/net/IPAddressTest.cpp
      src/jaegertracing/net/SocketTest.cpp
      src/jaegertracing/net/URITest.cpp
      src/jaegertracing/net/http/HeaderTest.cpp
      src/jaegertracing/net/http/MethodTest.cpp
      src/jaegertracing/net/http/ResponseTest.cpp
      src/jaegertracing/propagation/PropagatorTest.cpp
      src/jaegertracing/reporters/ReporterTest.cpp
      src/jaegertracing/samplers/SamplerTest.cpp
      src/jaegertracing/testutils/MockAgentTest.cpp
      src/jaegertracing/testutils/TracerUtil.cpp
      src/jaegertracing/testutils/TUDPTransportTest.cpp
      src/jaegertracing/utils/ErrorUtilTest.cpp
      src/jaegertracing/utils/RateLimiterTest.cpp
      src/jaegertracing/utils/UDPClientTest.cpp)
  add_executable(UnitTest ${TEST_SRC})
  target_compile_definitions(UnitTest PUBLIC
      GTEST_HAS_TR1_TUPLE=1
      GTEST_USE_OWN_TR1_TUPLE=0)
  target_link_libraries(
      UnitTest testutils GTest::main jaegertracing-static ${LIBS})
  add_test(NAME UnitTest COMMAND UnitTest)
  if(JAEGERTRACING_COVERAGE)
      setup_target_for_coverage(NAME UnitTestCoverage
                                EXECUTABLE UnitTest
                                DEPENDENCIES UnitTest)
  endif()
endif()

# Installation (https://github.com/forexample/package-example)

# Layout. This works for all platforms:
#   * <prefix>/lib/cmake/<PROJECT-NAME>
#   * <prefix>/lib/
#   * <prefix>/include/
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

# Configuration
set(version_config "${generated_dir}/${PROJECT_NAME}ConfigVersion.cmake")
set(project_config "${generated_dir}/${PROJECT_NAME}Config.cmake")
set(TARGETS_EXPORT_NAME "${PROJECT_NAME}Targets")
set(namespace "${PROJECT_NAME}::")

# Include module with fuction 'write_basic_package_version_file'
include(CMakePackageConfigHelpers)

# Configure '<PROJECT-NAME>ConfigVersion.cmake'
# Use:
#   * PROJECT_VERSION
write_basic_package_version_file(
  "${version_config}" COMPATIBILITY SameMajorVersion
)

# Configure '<PROJECT-NAME>Config.cmake'
# Use variables:
#   * TARGETS_EXPORT_NAME
#   * PROJECT_NAME
configure_package_config_file(
  "cmake/Config.cmake.in"
  "${project_config}"
  INSTALL_DESTINATION "${config_install_dir}"
)

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
#   * src/jaegertracing/Tracer.h -> <prefix>/include/jaegertracing/Tracer.h
install(DIRECTORY "src/jaegertracing"
        DESTINATION "${include_install_dir}"
        FILES_MATCHING
        PATTERN "*.h"
        PATTERN "testutils/*.h" EXCLUDE)

#   * build/src/jaegertracing/Constants.h ->
#     <prefix>/include/jaegertracing/Constants.h
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/src/jaegertracing/Constants.h"
        DESTINATION "${include_install_dir}/jaegertracing")

# Config
#   * <prefix>/lib/cmake/jaegertracing/jaegertracingConfig.cmake
#   * <prefix>/lib/cmake/jaegertracing/jaegertracingConfigVersion.cmake
install(
  FILES "${project_config}" "${version_config}"
  DESTINATION "${config_install_dir}"
)

# Config
#   * <prefix>/lib/cmake/jaegertracing/jaegertracingTargets.cmake
install(
  EXPORT "${TARGETS_EXPORT_NAME}"
  NAMESPACE "${namespace}"
  DESTINATION "${config_install_dir}"
)
