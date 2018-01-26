# Find jsoncpp
#
# Find the nlohmann json header
#
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#
#  nlohmann_json_INCLUDE_DIR, where to find header, etc.
#
#  nlohmann_json_FOUND, If false, do not try to use jsoncpp.
#
#  nlohmann_json_LIBRARIES, empty since no linkage is required, this
#      is a header-only library.
#
#  nlohmann_json_INCLUDE_NAME, the actual header name. You only have
#      to use this if you want to support 2.0.x which installs
#      a top-level json.hpp instead of nlohmann/json.hpp
#

# only look in default directories
set(nlohmann_json_INCLUDE_NAME "nlohmann/json.hpp")
find_path(
	nlohmann_json_INCLUDE_DIR
	NAMES "${nlohmann_json_INCLUDE_NAME}"
	DOC "nlohmann json include dir"
)

if (NOT nlohmann_json_INCLUDE_DIR)
	set(nlohmann_json_INCLUDE_NAME "json.hpp")
	find_path(
		nlohmann_json_INCLUDE_DIR
		NAMES "${nlohmann_json_INCLUDE_NAME}"
	)
endif()

set(nlohmann_json_INCLUDE_NAME ${nlohmann_json_INCLUDE_NAME} CACHE STRING "nlohmann header file name")

set(nlohmann_json_LIBRARIES NOTFOUND CACHE STRING "no library is required by nlohmann_json")

# Version detection. Unfortunately the header doesn't expose a proper version
# define.
if (nlohmann_json_INCLUDE_DIR AND nlohmann_json_INCLUDE_NAME)
	file(READ "${nlohmann_json_INCLUDE_DIR}/${nlohmann_json_INCLUDE_NAME}" NL_HDR_TXT LIMIT 1000)
	if (NL_HDR_TXT MATCHES "version ([0-9]+\.[0-9]+\.[0-9]+)")
		set(nlohmann_json_VERSION "${CMAKE_MATCH_1}")
	endif()
endif()

set(nlohmann_json_VERSION "${nlohmann_json_VERSION}" CACHE STRING "nlohmann header version")

# handle the QUIETLY and REQUIRED arguments and set nlohmann_json_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
	nlohmann_json
	REQUIRED_VARS nlohmann_json_INCLUDE_DIR nlohmann_json_INCLUDE_NAME
	VERSION_VAR nlohmann_json_VERSION)

add_library(nlohmann_json INTERFACE IMPORTED)

set_target_properties(nlohmann_json PROPERTIES
  INTERFACE_INCLUDE_DIRECTORIES "${nlohmann_json_INCLUDE_DIR}"
)
