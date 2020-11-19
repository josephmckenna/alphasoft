# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agdaq)

# Version numbering
set (basicApp_VERSION_MAJOR 1)
set (basicApp_VERSION_MAJOR 0)
set (CMAKE_CXX_FLAGS "-O2")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(DEFINED ENV{AGRELEASE})
   message("AGRELEASE set to " $ENV{AGRELEASE}) 
else()
   message(FATAL_ERROR "AGRELEASE not set! source agconfig.sh please")
endif()

find_package(Git REQUIRED)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
   option(GIT_SUBMODULE "Check submodules during build" ON)
endif()

option(BUILD_AG "Build AG" ON)
option(BUILD_AG_SIM "Build AG Simulation" OFF)
option(BUILD_A2 "Build A2" ON)

#Send these settings as precompiler flags (mostly used in rootUtils)
if (BUILD_AG)
   add_compile_definitions(BUILD_AG)
endif (BUILD_AG)
if (BUILD_A2)
   add_compile_definitions(BUILD_A2)
endif (BUILD_A2)

set(CMAKE_INSTALL_PREFIX "$ENV{AGRELEASE}/bin")
message("CMAKE_INSTALL_PREFIX = ${CMAKE_INSTALL_PREFIX} ")
# Enforce out-of-source builds 
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" _insource)
if(_insource)
   message(FATAL_ERROR "Error: In-source build detected! Please create a build directory!")
endif(_insource) 

# Checks $CLICOLOR_FORCE variable to force compiler to emit coloured output
# The parameter is based on compiler ID as it's not standardised
if ($ENV{CLICOLOR_FORCE})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(Clang|AppleClang)$")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()


function(get_submodule MODULE_NAME)
   if(GIT_SUBMODULE)
        message(STATUS "Submodule ${MODULE_NAME} update")
        execute_process(COMMAND ${GIT_EXECUTABLE} submodule update ${MODULE_NAME} --init
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
        if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
        endif()
    endif()
endfunction()

# Directories


#Universal libraries (Include some flow and TSpill stuff)
get_submodule(rootana)
add_subdirectory(rootana)
add_subdirectory(analib)

#rootUtils is customised based on above arguments
add_subdirectory(rootUtils)

#AG libraries
if (BUILD_AG)
   add_subdirectory(recolib)
   get_submodule(agana)
   add_subdirectory(agana)
   add_subdirectory(aged)
   add_subdirectory(ana)
   add_subdirectory(reco)
endif (BUILD_AG)
if (BUILD_AG_SIM)
   #get_submodule(simulation/garfieldpp)
   add_subdirectory(simulation/garfieldpp)
   #get_submodule(simulation/submodules/CADMesh)
   #add_subdirectory(simulation/submodules/CADMesh)
   add_subdirectory(simulation/geant4)
   add_subdirectory(simulation/agg4)
endif (BUILD_AG_SIM)

#A2 libraries
if (BUILD_A2)
   add_subdirectory(a2lib)
   add_subdirectory(alpha2)
endif (BUILD_A2)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
if(EXISTS $ENV{ROOTSYS}/ROOTConfig.cmake)
    if(NOT DEFINED ENV{CMAKE_PREFIX_PATH})
        list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
    endif()
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/etc/cmake")

# Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
