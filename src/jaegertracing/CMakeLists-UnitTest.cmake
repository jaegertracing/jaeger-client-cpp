cmake_minimum_required(VERSION 3.3)

include(CTest)
if(BUILD_TESTING)
  hunter_add_package(GTest)
  find_package(GTest ${hunter_config} REQUIRED)

  set(TEST_SRC
      ConfigTest.cpp
      ReferenceTest.cpp
      SpanContextTest.cpp
      SpanTest.cpp
      TagTest.cpp
      TraceIDTest.cpp
      TracerTest.cpp
      UDPTransportTest.cpp
      baggage/BaggageTest.cpp
      metrics/MetricsTest.cpp
      metrics/NullStatsFactoryTest.cpp
      net/IPAddressTest.cpp
      net/SocketTest.cpp
      net/URITest.cpp
      net/http/HeaderTest.cpp
      net/http/MethodTest.cpp
      net/http/ResponseTest.cpp
      propagation/PropagatorTest.cpp
      reporters/ReporterTest.cpp
      samplers/SamplerTest.cpp
      testutils/MockAgent.cpp
      testutils/TUDPTransport.cpp
      testutils/SamplingManager.cpp
      testutils/MockAgentTest.cpp
      testutils/TracerUtil.cpp
      testutils/TUDPTransportTest.cpp
      utils/ErrorUtilTest.cpp
      utils/RateLimiterTest.cpp
      utils/UDPClientTest.cpp)

  if(HUNTER_ENABLED)
    # Hunter uses a different target name for GTest than CMake's own packages do;
    # note the target name's case.
    list(APPEND TEST_LIBS GTest::main)
  else()
    list(APPEND TEST_LIBS GTest::Main)
  endif()

  find_package(Threads REQUIRED)
  list(APPEND TEST_LIBS Threads::Threads)

  # Shared test
  add_executable(UnitTest ${TEST_SRC})
  target_link_libraries(UnitTest ${TEST_LIBS} jaegertracing)

  # Also make sure static linkage works
  add_executable(UnitTestStatic ${TEST_SRC})
  target_link_libraries(UnitTestStatic ${TEST_LIBS} jaegertracing-static)

  foreach(tgt UnitTest UnitTestStatic)
    target_compile_definitions(${tgt} PUBLIC
        GTEST_HAS_TR1_TUPLE=0
        GTEST_USE_OWN_TR1_TUPLE=0)
    target_include_directories(${tgt} PRIVATE
      $<BUILD_INTERFACE:${jaegertracing_SOURCE_DIR}/src>
      $<BUILD_INTERFACE:${jaegertracing_BINARY_DIR}/src>)
    add_test(NAME ${tgt} COMMAND ${tgt})

    if(THREADS_HAVE_PTHREAD_ARG)
      target_compile_options(${tgt} PUBLIC "-pthread")
    endif()
    if(CMAKE_THREAD_LIBS_INIT)
      target_link_libraries(${tgt} "${CMAKE_THREAD_LIBS_INIT}")
    endif()

  endforeach(tgt)

  if(JAEGERTRACING_COVERAGE)
    set(COVERAGE_EXCLUDES "${CMAKE_CURRENT_SOURCE_DIR}/thrift-gen/*" "*Test.cpp")

    # Don't do coverage twice, just instrument one build
    if (JAEGER_BUILD_STATIC)
      setup_target_for_coverage(NAME UnitTestCoverage
          EXECUTABLE UnitTestStatic
          DEPENDENCIES UnitTestStatic)
    else()
      setup_target_for_coverage(NAME UnitTestCoverage
          EXECUTABLE UnitTest
          DEPENDENCIES UnitTest)
    endif()
  endif()

endif(BUILD_TESTING)

add_executable(shortlived "shortlived.cpp")
target_link_libraries(shortlived jaegertracing)
target_include_directories(shortlived PRIVATE
  $<BUILD_INTERFACE:${jaegertracing_SOURCE_DIR}/src>
  $<BUILD_INTERFACE:${jaegertracing_BINARY_DIR}/src>)

