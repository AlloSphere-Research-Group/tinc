cmake_minimum_required(VERSION 3.8)
project(TINCTests)

unset(BUILD_PYTHON_TESTS CACHE)
option(BUILD_PYTHON_TESTS "Build and run test for the C++ interaction with the python client" ON)

# Get and build googletest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

if(BUILD_PYTHON_TESTS)
  set(PYTHON_EXECUTABLE "python3" CACHE FILEPATH "Path to the python executable")
endif(BUILD_PYTHON_TESTS)

add_definitions(-DTINC_TESTS_TIMEOUT_MS=500)

# Build test binary
# file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.cpp)
set(TEST_SOURCES main.cpp
  processor.cpp
  parameters.cpp
  parameterspace.cpp
  tincprotocol_cache.cpp
  tincprotocol.cpp
  tincprotocol_parameters.cpp
  tincprotocol_parameterspaces.cpp
  tincprotocol_processors.cpp
  tincprotocol_diskbuffers.cpp
  tincprotocol_datapools.cpp
  tincprotocol_barrier.cpp
  tincprotocol_status.cpp
)

if(BUILD_PYTHON_TESTS)
  list(PREPEND TEST_SOURCES
    pythonclient_parameters.cpp
    python_common.cpp
    python_common.hpp
    pythonclient.cpp
    pythonclient_parameterspaces.cpp
    )
endif(BUILD_PYTHON_TESTS)

add_executable(tinc_tests ${TEST_SOURCES})

set_target_properties(tinc_tests PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_SOURCE_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_SOURCE_DIR}/bin
)

add_test(NAME tinc_tests COMMAND tinc_tests)

target_link_libraries(tinc_tests PUBLIC tinc gtest)
target_compile_definitions(tinc_tests PUBLIC
  "-DTINC_TESTS_SOURCE_DIR=\"${CMAKE_CURRENT_SOURCE_DIR}\""
  "-DPYTHON_EXECUTABLE=\"${PYTHON_EXECUTABLE}\"")

if (TINC_CODE_COVERAGE)
  set_target_properties(tinc_tests PROPERTIES COMPILE_FLAGS "-fPIE -g -O0 -Wall -fprofile-arcs -ftest-coverage")
  target_link_libraries(tinc_tests PUBLIC gcov)
endif()
