cmake_minimum_required(VERSION 3.3)

# See https://cmake.org/cmake/help/v3.3/manual/cmake-packages.7.html
# See https://rix0r.nl/blog/2015/08/13/cmake-guide/

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/cmake/toolchain.cmake"
    CACHE FILEPATH
    "Toolchain to use for building this package and dependencies")

include(CMakeDependentOption)
include(HunterGate)
HunterGate(
    URL "https://github.com/ruslo/hunter/archive/v0.19.174.tar.gz"
    SHA1 "712898528fb6dc8532454e2e323404d93a76d8ec"
    LOCAL
)

project(jaegertracing VERSION 0.3.0 LANGUAGES CXX)

# Layout. This works for all platforms:
#   * <prefix>/lib/cmake/<PROJECT-NAME>
#   * <prefix>/lib/
#   * <prefix>/include/
set(config_install_dir "lib/cmake/${PROJECT_NAME}")
set(include_install_dir "include")

set(generated_dir "${CMAKE_CURRENT_BINARY_DIR}/generated")

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

hunter_add_package(Boost COMPONENTS regex)
find_package(Boost ${hunter_config} REQUIRED regex)

hunter_add_package(thrift)
find_package(thrift 0.9.2 ${hunter_config} REQUIRED)
if(thrift_VERSION VERSION_GREATER "0.9.3")
  message(WARNING "jaeger-cpp is only known to work with Thrift 0.9.2 and Thrift 0.9.3")
endif()

hunter_add_package(opentracing-cpp)
# Not `${hunter_config}` because OpenTracing provides its own
# OpenTracingConfig.cmake file
find_package(OpenTracing ${hunter_config} REQUIRED)

hunter_add_package(nlohmann_json)
find_package(nlohmann_json 2.1.0 ${hunter_config} REQUIRED)

option(JAEGERTRACING_COVERAGE "Build with coverage" $ENV{COVERAGE})
option(JAEGERTRACING_BUILD_CROSSDOCK "Build crossdock" $ENV{CROSSDOCK})
cmake_dependent_option(
  JAEGERTRACING_WITH_YAML_CPP "Use yaml-cpp to parse config files" ON
  "NOT JAEGERTRACING_BUILD_CROSSDOCK" ON)

if(JAEGERTRACING_WITH_YAML_CPP)
  hunter_add_package(yaml-cpp)
  find_package(yaml-cpp ${hunter_config} REQUIRED)
endif()

if(JAEGERTRACING_COVERAGE)
    include(CodeCoverage)
endif()

# Installation and CMake package
#
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

if (BUILD_TESTING)
  enable_testing()
  add_custom_target(check COMMAND echo "run make test instead")
endif()

add_subdirectory(src/jaegertracing)
add_subdirectory(crossdock)
