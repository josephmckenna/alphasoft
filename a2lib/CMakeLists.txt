# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agdaq)

# Version numbering
set (basicApp_VERSION_MAJOR 1)
set (basicApp_VERSION_MAJOR 0)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

add_custom_target(
    sis_database ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/main.db
    )

add_custom_command(
    OUTPUT
        #${CMAKE_CURRENT_BINARY_DIR}/__header.h  # fake! ensure we run!
        ${CMAKE_CURRENT_SOURCE_DIR}/main.db    # real header, we write.
    # this command must generate: ${CMAKE_CURRENT_BINARY_DIR}/header.h
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/build_db.sh 
    )


# Directories
#add_subdirectory(legacy)
add_subdirectory(src)

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
find_package(ROOT REQUIRED COMPONENTS Minuit2 Geom)
# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# Include directories
include_directories(include include)
include_directories(include .) #Used for RootUtils
#include_directories(include $ENV{AGRELEASE}/recolib/include)
include_directories(include $ENV{AGRELEASE}/ana/include)
include_directories(include $ENV{AGRELEASE}/analib/include)
include_directories(include $ENV{ROOTANASYS}/include)
include_directories(include ${ROOT_INCLUDE_DIRS})
ROOT_GENERATE_DICTIONARY(a2lib include/*.h LINKDEF LibLinkDef.hh)

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx RootUtils/*.cxx)
file(GLOB C_SOURCES src/*.c)
add_library(alpha2 SHARED ${LIBRARY_SOURCES} ${C_SOURCES} a2lib.cxx )
find_library(LIBSQLITE3 sqlite3)
target_link_libraries(alpha2 ${ROOT_LIBRARIES} ${LIBSQLITE3})
