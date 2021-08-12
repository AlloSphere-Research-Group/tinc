cmake_minimum_required(VERSION 3.0)

project(tinc_server)

add_executable(tinc_server main.cpp)

target_link_libraries(tinc_server PRIVATE tinc)

set_target_properties(${tinc_server} PROPERTIES
  CXX_STANDARD 17
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_LIST_DIR}/bin
)
