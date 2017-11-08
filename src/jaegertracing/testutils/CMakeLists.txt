set(SRC
MockAgent.cpp
SamplingManager.cpp
TUDPTransport.cpp)

add_library(testutils ${SRC})
target_include_directories(testutils PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/src>)
target_link_libraries(testutils thrift::thrift_static)
