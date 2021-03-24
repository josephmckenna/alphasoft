#Typically the subdirectory should contain its own project() command invocation so that a full build system will be generated in the subdirectory
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alphasim)

#----------------------------------------------------------------------------
# Garfield++
#
if(EXISTS $ENV{GARFIELD_HOME}/install/lib/cmake/Garfield/GarfieldConfig.cmake)
  list(APPEND CMAKE_PREFIX_PATH $ENV{GARFIELD_HOME}/install/lib/cmake/Garfield)
endif()

find_package(Garfield REQUIRED)
if (GARFIELD_FOUND)
  message(STATUS "Garfield found...")
  message(STATUS "Garfield headers: ${GARFIELD_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "GARFILED NOT FOUND")
endif()


#----------------------------------------------------------------------------
set(CRY_DATA_PATH "${CMAKE_INSTALL_PREFIX}/cry/data/" CACHE INTERNAL "")
if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 
  add_definitions(-DCRY_DATA_PATH="${CRY_DATA_PATH}")
else()
  add_compile_definitions(CRY_DATA_PATH="${CRY_DATA_PATH}")
endif()
file(GLOB CRY_DATA_FILES cry/data/*.data)
install(FILES ${CRY_DATA_FILES} DESTINATION ${CRY_DATA_PATH})

message(STATUS "Simulation ${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/garfieldpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cry)

add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CADMesh)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/G4out)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/geant4)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/agg4)
