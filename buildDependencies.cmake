
# add allolib as a subdirectory to the project
add_subdirectory(external/allolib)

if (EXISTS ${CMAKE_CURRENT_LIST_DIR}/external/al_ext)
  message("Buiding extensions in al_ext")
  add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/external/al_ext)
  get_target_property(AL_EXT_LIBRARIES al_ext AL_EXT_LIBRARIES)
  target_link_libraries(tinc PUBLIC ${AL_EXT_LIBRARIES})
endif()

# json-schema-validator
set(BUILD_TESTS OFF CACHE BOOL "Tests for json-schema-validator" FORCE)
set(BUILD_EXAMPLES OFF CACHE BOOL "Examples for json-schema-validator" FORCE)
add_subdirectory(external/json-schema-validator)

set_target_properties(nlohmann_json_schema_validator PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
)

# link allolib to project
target_link_libraries(tinc PUBLIC al nlohmann_json_schema_validator)
target_include_directories(tinc PUBLIC ${TINC_INCLUDE_PATH} external/json-schema-validator/src external/PicoSHA2)

# For some reason the includes from allolib are not being passed to tinc. this does it manually
get_target_property(AL_INCLUDE_DIRS al INCLUDE_DIRECTORIES)
target_include_directories(tinc PUBLIC ${AL_INCLUDE_DIRS})

set(TINC_DEPS_INSTALL_PREFIX "${CMAKE_CURRENT_LIST_DIR}/external/usr/")

