cmake_minimum_required(VERSION 3.8)

project(TINC)

option(TINC_BUILD_EXAMPLES "Build TINC Examples" OFF)
option(TINC_BUILD_TESTS "Build TINC Unit Tests" OFF)
option(TINC_CODE_COVERAGE "Enable Code coverage for tests" OFF)
option(TINC_CPP_17 "Use C++17 features" ON)

set(TINC_SRC
    ${CMAKE_CURRENT_LIST_DIR}/src/CacheManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DataPool.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DataPoolNetCDF.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DataPoolJson.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DiskBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DiskBufferNetCDFData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DistributedPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/IdObject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpace.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpaceDimension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Processor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ProcessorGraph.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ProcessorCpp.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ProcessorAsyncWrapper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ProcessorScript.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/TincClient.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/TincProtocol.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/TincServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VASPReader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/tinc_protocol.pb.cc


    ${CMAKE_CURRENT_LIST_DIR}/src/vis/AtomRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vis/GUI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vis/SceneObject.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/vis/TrajectoryRender.cpp
  )

set(TINC_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include)

set(TINC_HEADERS
    ${TINC_INCLUDE_PATH}/tinc/BufferManager.hpp
    ${TINC_INCLUDE_PATH}/tinc/CacheManager.hpp
    ${TINC_INCLUDE_PATH}/tinc/DataPool.hpp
    ${TINC_INCLUDE_PATH}/tinc/DataPoolNetCDF.hpp
    ${TINC_INCLUDE_PATH}/tinc/DataPoolJson.hpp
    ${TINC_INCLUDE_PATH}/tinc/DeferredComputation.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBufferAbstract.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBufferImage.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBufferJson.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBufferNetCDFData.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBufferText.hpp
    ${TINC_INCLUDE_PATH}/tinc/DistributedPath.hpp
    ${TINC_INCLUDE_PATH}/tinc/IdObject.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpace.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpaceDimension.hpp
    ${TINC_INCLUDE_PATH}/tinc/PeriodicTask.hpp
    ${TINC_INCLUDE_PATH}/tinc/Processor.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorCpp.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorGraph.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorAsyncWrapper.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorScript.hpp
    ${TINC_INCLUDE_PATH}/tinc/TincClient.hpp
    ${TINC_INCLUDE_PATH}/tinc/TincProtocol.hpp
    ${TINC_INCLUDE_PATH}/tinc/TincServer.hpp
    ${TINC_INCLUDE_PATH}/tinc/VASPReader.hpp

    ${TINC_INCLUDE_PATH}/tinc/vis/AtomRenderer.hpp
    ${TINC_INCLUDE_PATH}/tinc/vis/GUI.hpp
    ${TINC_INCLUDE_PATH}/tinc/vis/SceneObject.hpp
    ${TINC_INCLUDE_PATH}/tinc/vis/TrajectoryRender.hpp
)


add_library(tinc ${TINC_SRC} ${TINC_HEADERS})

# binaries are put into the ./bin directory by default
if(TINC_CPP_17)
    set(C_STANDARD 17)
    set_target_properties(tinc PROPERTIES
      CXX_STANDARD 17
      CXX_STANDARD_REQUIRED ON
      )
  target_compile_definitions(tinc PUBLIC TINC_CPP_17)
else()
    set_target_properties(tinc PROPERTIES
      CXX_STANDARD 14
      CXX_STANDARD_REQUIRED ON
      )
    if (${CMAKE_SYSTEM_NAME} MATCHES "Windows")
        # Needed for PathIsRelative
        target_link_libraries(tinc PUBLIC cfgmgr32.lib Shlwapi.lib)
    endif()
endif(TINC_CPP_17)

set_target_properties(tinc PROPERTIES
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
)

# Protobuf dependency
set (TINC_ROOT ${CMAKE_CURRENT_LIST_DIR})

include(ExternalProject)
ExternalProject_Add(
  protobufProject
  PREFIX external/protobuf
  URL https://github.com/protocolbuffers/protobuf/archive/v3.6.1.tar.gz
  BINARY_DIR ${TINC_ROOT}/external/protobuf-build-${CMAKE_BUILD_TYPE}
  SOURCE_DIR ${TINC_ROOT}/external/protobuf
  CMAKE_CACHE_ARGS
    "-DCMAKE_BUILD_TYPE:STRING=${CMAKE_BUILD_TYPE}"
    "-Dprotobuf_BUILD_TESTS:BOOL=OFF"
    "-Dprotobuf_BUILD_EXAMPLES:BOOL=OFF"
    "-Dprotobuf_WITH_ZLIB:BOOL=OFF"
    "-Dprotobuf_BUILD_SHARED_LIBS:BOOL=OFF"
    "-Dprotobuf_MSVC_STATIC_RUNTIME:BOOL=OFF"
    "-DCMAKE_CXX_COMPILER:STRING=${CMAKE_CXX_COMPILER}"
    # other project specific parameters
  SOURCE_SUBDIR cmake
#  BUILD_COMMAND "@CMAKE_COMMAND@" .
  BUILD_ALWAYS ON
#  STEP_TARGETS build
  INSTALL_COMMAND ""
)

ExternalProject_Get_Property(protobufProject source_dir)
if(WIN32)
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(protobuflib libprotobufd)
    else()
        set(protobuflib libprotobuf)
    endif()
else()
    if (CMAKE_BUILD_TYPE STREQUAL "Debug")
        set(protobuflib protobufd)
    else()
        set(protobuflib protobuf)
    endif()
endif()

message("Using protobuf at: ${source_dir}")
target_include_directories(tinc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external/protobuf/src)
target_link_directories(tinc PUBLIC  ${CMAKE_CURRENT_LIST_DIR}/external/protobuf-build-${CMAKE_BUILD_TYPE})
target_link_libraries(tinc PUBLIC ${protobuflib})

add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/src/tinc_protocol.pb.cc
                   COMMAND ${CMAKE_CURRENT_LIST_DIR}/external/protobuf-build-${CMAKE_BUILD_TYPE}/protoc -I=${CMAKE_CURRENT_LIST_DIR}/external/protobuf/src  -I=${CMAKE_CURRENT_LIST_DIR}/src --cpp_out=${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src/tinc_protocol.proto
                   DEPENDS protobufProject ${CMAKE_CURRENT_LIST_DIR}/src/tinc_protocol.proto
)

add_custom_command(OUTPUT ${CMAKE_CURRENT_LIST_DIR}/tinc-python/tinc/tinc_protocol_pb2.py
                   COMMAND ${CMAKE_CURRENT_LIST_DIR}/external/protobuf-build-${CMAKE_BUILD_TYPE}/protoc -I=${CMAKE_CURRENT_LIST_DIR}/external/protobuf/src -I=${CMAKE_CURRENT_LIST_DIR}/src --python_out=${CMAKE_CURRENT_LIST_DIR}/tinc-python/tinc ${CMAKE_CURRENT_LIST_DIR}/src/tinc_protocol.proto
                   DEPENDS protobufProject ${CMAKE_CURRENT_LIST_DIR}/src/tinc_protocol.proto
)

add_custom_target(create_protobuf_files ALL
    DEPENDS ${CMAKE_CURRENT_LIST_DIR}/src/tinc_protocol.pb.cc ${CMAKE_CURRENT_LIST_DIR}/tinc-python/tinc/tinc_protocol_pb2.py)
add_dependencies(tinc create_protobuf_files)


execute_process(COMMAND ${CMAKE_CURRENT_BINARY_DIR}/protobuf/protoc -I=${CMAKE_CURRENT_LIST_DIR}/src --cpp_out=${CMAKE_CURRENT_LIST_DIR}/src ${CMAKE_CURRENT_LIST_DIR}/src/tinc.proto)


##### Binary dependencies
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/external/usr)

if(WIN32)
    set(NETCDF4_INSTALL_ROOT "C:/Program Files/netCDF 4.7.4/" CACHE STRING
        "Path to installation directory of netCDF4.")
else()
    # TODO run build_deps.sh script if netcdf not found.
    set(NETCDF4_INSTALL_ROOT "" CACHE STRING
        "Path to installation directory of netCDF4. Leave blanc if using build_deps script")
endif()

if (NOT NETCDF4_INSTALL_ROOT STREQUAL "")

    set(HDF5_INCLUDE_DIRS "C:/Program Files/netCDF 4.7.4/include")

    set(HDF5_LIBRARIES "C:/Program Files/netCDF 4.7.4/lib/hdf5_cpp.lib"
        "C:/Program Files/netCDF 4.7.4/lib/libhdf5_hl_cpp.lib"
        "C:/Program Files/netCDF 4.7.4/lib/libhdf5_tools.lib")
    set(HDF5_FOUND ON)

    FIND_LIBRARY(NETCDF_LIBRARY netCDF
      PATHS
      "${CMAKE_CURRENT_LIST_DIR}/external/usr/lib"
      /usr/lib
      "C:/Program Files/netCDF 4.7.4/lib"
      )

    find_file(NETCDF_HEADER netcdf.h
      PATHS
      "${CMAKE_CURRENT_LIST_DIR}/external/usr/include"
      /usr/lib
      "C:/Program Files/netCDF 4.7.4/include"
      )

    get_filename_component(NETCDF_INCLUDE_DIRS ${NETCDF_HEADER} DIRECTORY)
else()
    FIND_PACKAGE(HDF5 COMPONENTS C HL REQUIRED)
	FIND_LIBRARY(NETCDF_LIBRARY NAMES netCDF netcdf
      PATHS
      ${CMAKE_CURRENT_LIST_DIR}/external/usr/lib
      )
endif()

if(NETCDF_LIBRARY AND HDF5_FOUND)
  message("Using NetCDF: ${NETCDF_LIBRARY}")
  target_compile_definitions(tinc PUBLIC -DTINC_HAS_NETCDF)
  target_include_directories(tinc PUBLIC "${NETCDF_INCLUDE_DIRS}")
  target_link_libraries(tinc PUBLIC ${NETCDF_LIBRARY})
  target_link_directories(tinc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external/usr/lib)

  message("Using HDF5 root: ${HDF5_LIBRARIES}")
  target_compile_definitions(tinc PUBLIC -DTINC_HAS_HDF5)
  target_link_libraries(tinc PUBLIC ${HDF5_LIBRARIES})
  target_link_directories(tinc PUBLIC ${CMAKE_CURRENT_LIST_DIR}/external/usr/lib)
else()
    message("NetCDF4 not found. Not using netcdf or HDF5.")
endif(NETCDF_LIBRARY AND HDF5_FOUND)

##### In tree build dependencies
include(buildDependencies.cmake)

##### tinc_server app
add_subdirectory(tinc_server)

##### Examples
if(TINC_BUILD_EXAMPLES)
  message("Building TINC Examples")
  add_subdirectory(examples)
  add_subdirectory(cookbook)
endif(TINC_BUILD_EXAMPLES)

if(TINC_BUILD_TESTS)
  message("Building TINC Tests")
  
  if (TINC_CODE_COVERAGE)
    message("Building code coverage for tests")
    set_target_properties(tinc PROPERTIES COMPILE_FLAGS "-g -O0 -Wall -fprofile-arcs -ftest-coverage")
    target_link_libraries(tinc PUBLIC gcov)
  endif()
  
  add_subdirectory(tests)
endif(TINC_BUILD_TESTS)
