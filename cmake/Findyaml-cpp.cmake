# Find the yaml-cpp header and libraries
#
# if you nee to add a custom library search path, do it via via CMAKE_PREFIX_PATH
#
# This module defines
#  yaml-cpp_INCLUDE_DIR, where to find header, etc.
#  yaml-cpp_LIBRARIES, libraries to link to use yaml-cpp
#  yaml-cpp_FOUND, If false, do not try to use yaml-cpp.
#

# only look in default directories
find_path(
  yaml-cpp_INCLUDE_DIR
  NAMES yaml-cpp/yaml.h
  DOC "yaml-cpp include dir"
)

find_library(yaml-cpp_LIBRARIES
  NAMES yaml-cpp
  PATH_SUFFIXES lib lib64)

# handle the QUIETLY and REQUIRED arguments and set yaml-cpp_FOUND to TRUE
# if all listed variables are TRUE, hide their existence from configuration view
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(yaml-cpp DEFAULT_MSG yaml-cpp_LIBRARIES yaml-cpp_INCLUDE_DIR)

# See https://cmake.org/cmake/help/v3.3/manual/cmake-packages.7.html
# See https://rix0r.nl/blog/2015/08/13/cmake-guide/

IF (yaml-cpp_LIBRARIES AND yaml-cpp_INCLUDE_DIR)
  # This won't work right on Windows if we find a .dll; we have to find the
  # separate import library and set IMPORTED_IMPLIB to the .lib associated with
  # the .DLL
  ADD_LIBRARY(yaml-cpp::yaml-cpp UNKNOWN IMPORTED)
  SET_TARGET_PROPERTIES(yaml-cpp::yaml-cpp PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    INTERFACE_INCLUDE_DIRECTORIES "${yaml-cpp_INCLUDE_DIR}"
    IMPORTED_LOCATION "${yaml-cpp_LIBRARIES}"
  )
ENDIF()
