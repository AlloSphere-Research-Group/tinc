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

# Build test binary
file(GLOB_RECURSE TEST_SOURCES LIST_DIRECTORIES false *.cpp)

add_executable(tinc_tests ${TEST_SOURCES})

add_test(NAME tinc_tests COMMAND tinc_tests)

target_link_libraries(tinc_tests PUBLIC tinc gtest)
