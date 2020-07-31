cmake_minimum_required(VERSION 3.8)

project(TINC)

option(TINC_BUILD_EXAMPLES "Build TINC Examples" OFF)

set(TINC_SRC

    ${CMAKE_CURRENT_LIST_DIR}/src/AtomRenderer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ComputationChain.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/CppProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/DiskBuffer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/GUI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpace.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpaceDimension.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ParameterSpaceNode.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/Processor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ProcessorAsync.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/ScriptProcessor.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/TincServer.cpp
    ${CMAKE_CURRENT_LIST_DIR}/src/VASPReader.cpp
  )

set(TINC_INCLUDE_PATH ${CMAKE_CURRENT_LIST_DIR}/include)

set(TINC_HEADERS
    ${TINC_INCLUDE_PATH}/tinc/AtomRenderer.hpp
    ${TINC_INCLUDE_PATH}/tinc/BufferManager.hpp
    ${TINC_INCLUDE_PATH}/tinc/ComputationChain.hpp
    ${TINC_INCLUDE_PATH}/tinc/CppProcessor.hpp
    ${TINC_INCLUDE_PATH}/tinc/DeferredComputation.hpp
    ${TINC_INCLUDE_PATH}/tinc/DiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/GUI.hpp
    ${TINC_INCLUDE_PATH}/tinc/ImageDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/JsonDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/NetCDFDiskBuffer.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpace.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpaceDimension.hpp
    ${TINC_INCLUDE_PATH}/tinc/ParameterSpaceNode.hpp
    ${TINC_INCLUDE_PATH}/tinc/PeriodicTask.hpp
    ${TINC_INCLUDE_PATH}/tinc/Processor.hpp
    ${TINC_INCLUDE_PATH}/tinc/ProcessorAsync.hpp
    ${TINC_INCLUDE_PATH}/tinc/ScriptProcessor.hpp
    ${TINC_INCLUDE_PATH}/tinc/TincServer.hpp
    ${TINC_INCLUDE_PATH}/tinc/VASPReader.hpp
)

add_library(tinc ${TINC_SRC} ${TINC_HEADERS})

# binaries are put into the ./bin directory by default
set_target_properties(tinc PROPERTIES
  CXX_STANDARD 14
  CXX_STANDARD_REQUIRED ON
  RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_CURRENT_LIST_DIR}/bin
  RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_CURRENT_LIST_DIR}/bin
)

##### Binary dependencies
set(CMAKE_PREFIX_PATH ${CMAKE_CURRENT_LIST_DIR}/external/usr)

if(WIN32)
    set(NETCDF4_INSTALL_ROOT "C:/Program Files/netCDF 4.7.4/" CACHE STRING
        "Path to installation directory of netCDF4.")
else()
    set(NETCDF4_INSTALL_ROOT "" CACHE STRING
        "Path to installation directory of netCDF4.")
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

##### Examples
if(TINC_BUILD_EXAMPLES)
  message("Building TINC Examples")
add_subdirectory(examples)
add_subdirectory(cookbook)
endif(TINC_BUILD_EXAMPLES)
