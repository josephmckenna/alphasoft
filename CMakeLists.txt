# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)

# Stop CMAKE as soon as possible if AGRELEASE is not set
if(DEFINED ENV{AGRELEASE})
   message("AGRELEASE set to " $ENV{AGRELEASE}) 
else()
   message(FATAL_ERROR "AGRELEASE not set! source agconfig.sh please")
endif()

project(alphasoft)

# Enforce out-of-source builds 
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" _insource)
if(_insource)
   # execute_process(COMMAND rm -vf $ENV{AGRELEASE}/CMakeCache.txt) This doesn't work... CMakeCache.txt gets updated after delete
   message(FATAL_ERROR "Error: In-source build detected! Please create a build directory! And please delete: $ENV{AGRELEASE}/CMakeCache.txt")
   
endif(_insource) 

# Version numbering
set(${PROJECT_NAME}_VERSION_MAJOR 1)
set(${PROJECT_NAME}_VERSION_MAJOR 0)
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE "RelWithDebInfo" CACHE STRING
            "Default build type: RelWithDebInfo" FORCE)
endif()

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
  set(CMAKE_INSTALL_PREFIX "$ENV{AGRELEASE}/bin" CACHE PATH "..." FORCE)
endif()



# IDentify OS (linux, windows, etc.)
string(REGEX MATCH ^[a-zA-Z]* OS_TYPE ${CMAKE_SYSTEM})
string(TOLOWER ${OS_TYPE} OS_TYPE)
add_compile_definitions(OSTYPE="${OS_TYPE}")
message(STATUS "OS: ${CMAKE_SYSTEM} : ${OS_TYPE}")

# Project specific option
set(PRE_CONFIG "" CACHE STRING "Pre-config script, used to build setup.{sh,py,C} (useful for external environment config scripts)")
option(BUILD_AG "Build AG" ON)
option(BUILD_AG_SIM "Build AG Simulation" OFF)
option(BUILD_A2 "Build A2" ON)
option(BUILD_A2_SIM "Build A2 Simulation" OFF)
option(BUILD_MANALYZER "Build MIDAS analyzer (manalyzer)" OFF)
option(WARNINGS_ALL "Enable gcc warnings" OFF)
option(LINTER "Enable clang-tidy to inspect C++ code" OFF)


# Find Git
find_package(Git REQUIRED)
if(GIT_FOUND AND EXISTS "${PROJECT_SOURCE_DIR}/.git")
   option(GIT_SUBMODULE "Check submodules during build" ON)
endif()

# Find MIDAS
if(DEFINED ENV{MIDASSYS})
   include($ENV{MIDASSYS}/lib/midas-targets.cmake)
   message(STATUS "${PROJECT_NAME}: Building with MIDAS from $ENV{MIDASSYS}/lib/midas-targets.cmake")
   set(MIDAS_FOUND TRUE)
   set(MIDASSYS "$ENV{MIDASSYS}")
   set(MIDAS_LIBRARIES midas)
   message(STATUS "MIDAS found ${MIDASSYS}")
   message(STATUS "[${PROJECT_NAME}] MIDAS_LIBRARIES = ${MIDAS_LIBRARIES}")
   message(STATUS "[${PROJECT_NAME}] MIDAS_LIBRARY_DIRS = ${MIDAS_LIBRARY_DIRS}")
else()
   message(STATUS "${PROJECT_NAME}: Building without MIDAS")
   set(BUILD_MANALYZER ON)
   unset(MIDAS_FOUND)
endif()


# SQLITE3
find_package(SQLite3)
if(SQLite3_FOUND)
  message(STATUS "SQLite3 ${SQLite3_VERSION} found")
else()
  message(FATAL "SQLite3 NOT FOUND")
endif()

# Zlib
set(HAVE_LIBZ 0)
find_package(ZLIB)
if(ZLIB_FOUND)
    # TODO: rename HAVE_LIBZ to HAVE_ZLIB
    # (original rename was introduced in midasio commit f185f999
    # for compatibility with autotools which is not used)
    set(HAVE_LIBZ 1)
    set(HAVE_ZLIB 1)
    add_compile_definitions(HAVE_LIBZ HAVE_ZLIB)
endif()


# Find ROOT
if(EXISTS $ENV{ROOTSYS}/ROOTConfig.cmake)
    if(NOT DEFINED ENV{CMAKE_PREFIX_PATH})
        list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
    endif()
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/cmake")
# Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS MathMore Minuit Minuit2 XMLParser XMLIO Spectrum Gui Geom RHTTP)
# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
message(STATUS "ROOT headers: ${ROOT_INCLUDE_DIRS}")

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
      if(EXISTS "${CMAKE_SOURCE_DIR}/${MODULE_NAME}/.git")
         message("${MODULE_NAME} submodule ok")
      else()
         message(STATUS "Submodule ${MODULE_NAME} update --init")
         execute_process(COMMAND ${GIT_EXECUTABLE} submodule update --init ${MODULE_NAME}
                        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                        RESULT_VARIABLE GIT_SUBMOD_RESULT)
         if(NOT GIT_SUBMOD_RESULT EQUAL "0")
            message(FATAL_ERROR "git submodule update --init failed with ${GIT_SUBMOD_RESULT}, please checkout submodules")
         endif()
      endif()
    endif()
endfunction()


if (LINTER)
   set(CMAKE_CXX_CLANG_TIDY
      clang-tidy;
      -header-filter=$ENV{AGREASE}/;
      -format-style=file;
      #In root 6.24.00, the TObject constructor triggers clang-tidy warnings
      -line-filter=[{"name":"TStorage.h","lines":[[9999999,9999999]]}]
      -checks='-*,clang-analyzer-*,-clang-analyzer-alpha*,bugprone*,modernize-*';
      #-warnings-as-errors=*;
   )
endif(LINTER)
include_directories(SYSTEM ${ROOT_INCLUDE_DIRS})


# Directories
set( _AGANA_STATIC TRUE)

#Send these settings as precompiler flags (mostly used in rootUtils)
if (BUILD_AG)
   if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 
      add_definitions(-DBUILD_AG)
   else()
      add_compile_definitions(BUILD_AG)
   endif()
   message(STATUS "Building AG software")
   file(WRITE ${CMAKE_BINARY_DIR}/BuildConfig.h "#define BUILD_AG\n")
else()
   file(WRITE ${CMAKE_BINARY_DIR}/BuildConfig.h "//#define BUILD_AG\n")
endif (BUILD_AG)
if (BUILD_A2)
   if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 
      add_definitions(-DBUILD_A2)
   else()
      add_compile_definitions(BUILD_A2)
   endif()
   message(STATUS "Building A2 software")
   file(APPEND ${CMAKE_BINARY_DIR}/BuildConfig.h "#define BUILD_A2\n")
else()
   file(APPEND ${CMAKE_BINARY_DIR}/BuildConfig.h "//#define BUILD_A2\n")
endif (BUILD_A2)

if (BUILD_AG_SIM)
   if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 
      add_definitions(-DBUILD_AG_SIM)
   else()
      add_compile_definitions(BUILD_AG_SIM)
   endif()
   message(STATUS "Building AG simulation software")
   file(APPEND ${CMAKE_BINARY_DIR}/BuildConfig.h "#define BUILD_AG_SIM\n")
else()
   file(APPEND ${CMAKE_BINARY_DIR}/BuildConfig.h "//#define BUILD_AG_SIM\n")
endif(BUILD_AG_SIM)
if (BUILD_A2_SIM)
   if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 
      add_definitions(-DBUILD_A2_SIM)
   else()
      add_compile_definitions(BUILD_A2_SIM)
   endif()
   message(STATUS "Building A2 simulation software")
   file(APPEND ${CMAKE_BINARY_DIR}/BuildConfig.h "#define BUILD_A2_SIM\n")
else()
   file(APPEND ${CMAKE_BINARY_DIR}/BuildConfig.h "//#define BUILD_A2_SIM\n")
endif(BUILD_A2_SIM)

if(WARNINGS_ALL)
  add_compile_options(-Wall -Wextra -pedantic)
endif(WARNINGS_ALL)

add_compile_options(-DALPHASOFT_DB_INSTALL_PATH=\"${CMAKE_INSTALL_PREFIX}/db\")
install(FILES ${CMAKE_BINARY_DIR}/BuildConfig.h DESTINATION include)

add_subdirectory(cmake_extras)

get_submodule(libGEM)
add_subdirectory(libGEM)

add_subdirectory(analib)

#rootUtils is customised based on above arguments (BUILD_A2 / BUILD_AG)
add_subdirectory(rootUtils)

if(BUILD_MANALYZER)
  message(STATUS "Cloning submoduels")
  # get midas-analyzer submodules
  get_submodule(midasio)
  get_submodule(mvodb)
  get_submodule(mjson)
  get_submodule(mxml)
  get_submodule(manalyzer)
  message(STATUS "Ready to build manalyzer")
endif()

#AG libraries
if (BUILD_AG)
   add_subdirectory(recolib)
   get_submodule(agana)
   add_subdirectory(agana)
   add_subdirectory(aged)
   add_subdirectory(ana)
   add_subdirectory(reco)
endif (BUILD_AG)

# AG sim
if (BUILD_AG_SIM)
  get_submodule(simulation/cry)
  get_submodule(simulation/CADMesh)
  add_subdirectory(simulation)
endif (BUILD_AG_SIM)

#A2 libraries
if (BUILD_A2)
   add_subdirectory(a2lib)
   add_subdirectory(alpha2)
endif (BUILD_A2)

if (BUILD_A2_SIM)
   add_subdirectory(simulation/a2mc)
endif(BUILD_A2_SIM)


message(STATUS "Installation path: ${CMAKE_INSTALL_PREFIX} ")

