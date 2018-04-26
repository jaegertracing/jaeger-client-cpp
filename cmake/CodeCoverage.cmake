# Copyright (c) 2012 - 2017, Lars Bilke
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without modification,
# are permitted provided that the following conditions are met:
#
# 1. Redistributions of source code must retain the above copyright notice, this
#    list of conditions and the following disclaimer.
#
# 2. Redistributions in binary form must reproduce the above copyright notice,
#    this list of conditions and the following disclaimer in the documentation
#    and/or other materials provided with the distribution.
#
# 3. Neither the name of the copyright holder nor the names of its contributors
#    may be used to endorse or promote products derived from this software without
#    specific prior written permission.
#
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
# ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
# WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
# DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
# ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
# (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
# LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
# ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
# SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

include(CMakeParseArguments)

# Check prereqs
find_program(LCOV_PATH lcov)
find_program(GENHTML_PATH genhtml)

if(CMAKE_C_COMPILER_ID MATCHES "GNU")
  find_program(GCOV_PATH gcov)
  if(NOT GCOV_PATH)
    message(FATAL_ERROR "gcov not found! Aborting...")
  endif()
  set(GCOV_TOOL ${GCOV_PATH})
elseif(CMAKE_C_COMPILER_ID MATCHES "[Cc]lang")
  if("${CMAKE_C_COMPILER_VERSION}" VERSION_LESS 3)
    message(FATAL_ERROR "Clang version must be 3.0.0 or greater! Aborting...")
  endif()
  set(extra_coverage_flags "-Qunused-arguments")
  find_program(LLVM_COV_PATH llvm-cov)
  if(NOT LLVM_COV_PATH)
    message(FATAL_ERROR "llvm-cov not found! Aborting...")
  endif()
  set(GCOV_TOOL "${CMAKE_CURRENT_SOURCE_DIR}/scripts/llvm-gcov.sh")
else()
  message(FATAL_ERROR "Compiler is not gcc nor clang! Aborting...")
endif()

set(COVERAGE_COMPILER_FLAGS "${extra_coverage_flags} -g -O0 --coverage -fprofile-arcs -ftest-coverage"
  CACHE INTERNAL "")

if(NOT CMAKE_BUILD_TYPE STREQUAL "Debug")
  message(WARNING "Code coverage results with an optimised (non-Debug) build may be misleading")
endif() # NOT CMAKE_BUILD_TYPE STREQUAL "Debug"

if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
  link_libraries(gcov)
else()
  set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} --coverage")
endif()

# Defines a target for running and collection code coverage information
# Builds dependencies, runs the given executable and outputs reports.
# NOTE! The executable should always have a ZERO as exit code otherwise
# the coverage generation will not complete.
#
# setup_target_for_coverage(
#     NAME testrunner_coverage                    # New target name
#     EXECUTABLE testrunner -j ${PROCESSOR_COUNT} # Executable in PROJECT_BINARY_DIR
#     DEPENDENCIES testrunner                     # Dependencies to build first
# )
function(setup_target_for_coverage)
  set(options NONE)
  set(oneValueArgs NAME)
  set(multiValueArgs EXECUTABLE EXECUTABLE_ARGS DEPENDENCIES)
  cmake_parse_arguments(Coverage "${options}" "${oneValueArgs}" "${multiValueArgs}" ${ARGN})

  if(NOT LCOV_PATH)
    message(FATAL_ERROR "lcov not found! Aborting...")
  endif() # NOT LCOV_PATH

  if(NOT GENHTML_PATH)
    message(FATAL_ERROR "genhtml not found! Aborting...")
  endif() # NOT GENHTML_PATH

  foreach(exe ${Coverage_EXECUTABLE})
    list(APPEND exe_commands COMMAND ${exe})
  endforeach()

  set(LCOV_INVOKE ${LCOV_PATH} --gcov-tool "${GCOV_TOOL}")

  # Setup target
  add_custom_target(${Coverage_NAME}

    # Cleanup lcov
    COMMAND  ${LCOV_INVOKE} --directory . --zerocounters

    # Run tests
    ${exe_commands}

    # Capturing lcov counters and generating report
    COMMAND ${LCOV_INVOKE} --base-directory ${CMAKE_SOURCE_DIR} --directory . --capture --output-file ${Coverage_NAME}.info --no-external
    COMMAND ${LCOV_INVOKE} --remove ${Coverage_NAME}.info ${COVERAGE_EXCLUDES} --output-file ${Coverage_NAME}.info.cleaned
    COMMAND ${GENHTML_PATH} --legend --demangle-cpp -o ${Coverage_NAME} ${Coverage_NAME}.info.cleaned
    COMMAND ${CMAKE_COMMAND} -E rename ${Coverage_NAME}.info.cleaned coverage.info
    COMMAND ${CMAKE_COMMAND} -E remove ${Coverage_NAME}.info

    WORKING_DIRECTORY ${PROJECT_BINARY_DIR}
    DEPENDS ${Coverage_DEPENDENCIES}
    COMMENT "Resetting code coverage counters to zero.\nProcessing code coverage counters and generating report."
  )

  # Show info where to find the report
  add_custom_command(TARGET ${Coverage_NAME} POST_BUILD
    COMMAND ;
    COMMENT "Open ./${Coverage_NAME}/index.html in your browser to view the coverage report."
  )
endfunction() # setup_target_for_coverage

function(append_coverage_compiler_flags flags_var)
  set(${flags_var} "${${flags_var}} ${COVERAGE_COMPILER_FLAGS}" PARENT_SCOPE)
  message(STATUS "Appending code coverage compiler flags: ${COVERAGE_COMPILER_FLAGS}")
endfunction() # append_coverage_compiler_flags
