cmake_minimum_required(VERSION 3.3)

project(jaegertracing_crossdock LANGUAGES CXX)

if(JAEGERTRACING_BUILD_CROSSDOCK)
  add_executable(crossdock Server.cpp)
  target_include_directories(crossdock PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}>)
  target_link_libraries(crossdock PUBLIC jaegertracing-static)

  set(JAEGER_CROSSDOCK_URL
"https://raw.githubusercontent.com/jaegertracing/jaeger/master/docker-compose/\
jaeger-docker-compose.yml")
  file(DOWNLOAD ${JAEGER_CROSSDOCK_URL} "${CMAKE_CURRENT_BINARY_DIR}/jaeger-docker-compose.yml")
  find_program(DOCKER_COMPOSE_EXE docker-compose)
  if (NOT DOCKER_COMPOSE_EXE)
    message(WARNING "cannot build crossdock support requested by -DJAEGERTRACING_BUILD_CROSSDOCK: docker-compose not found")
  else()
    set(DOCKER_COMPOSE_CMD ${DOCKER_COMPOSE_EXE}
        -f "${CMAKE_CURRENT_SOURCE_DIR}/docker-compose.yml"
        -f "${CMAKE_CURRENT_BINARY_DIR}/jaeger-docker-compose.yml")
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
    message(STATUS "crossdock enabled, use 'make crossdock-run'")
  endif()
endif()
