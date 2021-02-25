#Typically the subdirectory should contain its own project() command invocation so that a full build system will be generated in the subdirectory
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alphasim)

#----------------------------------------------------------------------------
# Garfield++
#
if(EXISTS $ENV{GARFIELD_HOME}/lib/cmake/Garfield/GarfieldConfig.cmake)
  list(APPEND CMAKE_PREFIX_PATH $ENV{GARFIELD_HOME}/lib/cmake/Garfield)
endif()

find_package(Garfield REQUIRED)
if (Garfield_FOUND)
  message(STATUS "Garfield found...")
  message(STATUS "Garfield headers: ${GARFIELD_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "GARFILED NOT FOUND")
endif()


#----------------------------------------------------------------------------

message(STATUS "Simulation ${CMAKE_CURRENT_SOURCE_DIR}")
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/common)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/garfieldpp)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/cry)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/CADMesh)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/G4out)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/geant4)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/agg4)
