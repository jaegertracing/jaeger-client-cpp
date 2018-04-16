cmake_minimum_required(VERSION 3.3)
PROJECT(cdemo C CXX)

add_executable(cdemo cdemo.c cdemo_tracing.cpp)

target_link_libraries(cdemo ${LIBS} jaegertracing)

target_include_directories(cdemo PRIVATE
	$<BUILD_INTERFACE:${jaegertracing_SOURCE_DIR}/src>
	$<BUILD_INTERFACE:${jaegertracing_BINARY_DIR}/src>)
