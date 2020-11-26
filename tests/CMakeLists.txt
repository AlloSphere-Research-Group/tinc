cmake_minimum_required(VERSION 3.8)
project(TINCTests)

# Get and build googletest
include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG        release-1.10.0
)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

add_definitions(-DTINC_TESTS_TIMEOUT_MS=250)
# Build test binary
# file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.cpp)
set(TEST_SOURCES main.cpp
  scriptprocessor.cpp
  tincprotocol.cpp
  tincprotocol_parameters.cpp
  tincprotocol_remote_parameter.cpp
  parameterspace.cpp
  tincprotocol_parameterspaces.cpp
  tincprotocol_processors.cpp
  tincprotocol_diskbuffers.cpp
  tincprotocol_datapools.cpp
)

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
