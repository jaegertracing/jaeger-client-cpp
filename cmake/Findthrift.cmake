# Find Thrift library and headers
#
# Sets:
#
#   thrift_FOUND
#   thrift_INCLUDE_DIR
#   thrift_LIBRARIES
#   thrift_VERSION
#
# Component libraries not currently detected separately.
#
# Cut down from https://github.com/facebookarchive/fblualib

CMAKE_MINIMUM_REQUIRED(VERSION 3.0.0 FATAL_ERROR)

FIND_PATH(thrift_INCLUDE_DIR "thrift/Thrift.h")

FIND_LIBRARY(thrift_LIBRARY thrift)
FIND_LIBRARY(thrift_LIBRARY_STATIC libthrift.a thrift.a)

SET(thrift_LIBRARIES ${thrift_LIBRARY} CACHE STRING "main thrift library")

MARK_AS_ADVANCED(thrift_LIBRARY thrift_LIBRARY_STATIC)

FILE(READ "${thrift_INCLUDE_DIR}/thrift/config.h" thrift_CONFIG_H)
IF (thrift_CONFIG_H MATCHES "#define PACKAGE_VERSION \"([0-9]+\\.[0-9]+\\.[0-9]+)\"")
    SET(thrift_VERSION "${CMAKE_MATCH_1}")
ELSEIF (thrift_CONFIG_H MATCHES "#define VERSION \"([0-9]+\\.[0-9]+\\.[0-9]+)\"")
    SET(thrift_VERSION "${CMAKE_MATCH_1}")
ENDIF()

SET(thrift_VERSION "${thrift_VERSION}" CACHE STRING "thrift library version")

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(
  thrift
  REQUIRED_VARS thrift_INCLUDE_DIR thrift_LIBRARIES
  VERSION_VAR thrift_VERSION)

# See https://cmake.org/cmake/help/v3.3/command/add_library.html
# and https://cmake.org/cmake/help/v3.3/manual/cmake-packages.7.html

IF (thrift_LIBRARY_STATIC AND thrift_INCLUDE_DIR)
  ADD_LIBRARY(thrift::thrift_static STATIC IMPORTED)
  SET_TARGET_PROPERTIES(thrift::thrift_static PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    INTERFACE_INCLUDE_DIRECTORIES "${thrift_INCLUDE_DIR}"
    IMPORTED_LOCATION "${thrift_LIBRARY_STATIC}"
    )
ENDIF()

# This won't work right on Windows if we find a .dll; we have to find the
# separate import library and set IMPORTED_IMPLIB to the .lib associated with
# the .DLL
IF (thrift_LIBRARY AND thrift_INCLUDE_DIR)
  ADD_LIBRARY(thrift::thrift UNKNOWN IMPORTED)
  SET_TARGET_PROPERTIES(thrift::thrift PROPERTIES
    IMPORTED_LINK_INTERFACE_LANGUAGES "CXX"
    INTERFACE_INCLUDE_DIRECTORIES "${thrift_INCLUDE_DIR}"
    IMPORTED_LOCATION "${thrift_LIBRARY}"
  )
ENDIF()
