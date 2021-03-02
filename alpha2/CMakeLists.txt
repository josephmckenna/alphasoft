# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alphasoft)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


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

# Directories
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
find_package(ROOT REQUIRED COMPONENTS Minuit2 Spectrum)
link_libraries(${ROOT_LIBRARIES})

##Horrible horrible hack to get the libs for rootana (from rootana/include/rootana_cflags.txt )
list(APPEND ROOTANALIBS z Gui Core Imt RIO Net Hist Graf Graf3d Gpad ROOTVecOps Tree TreePlayer Rint Postscript Matrix Physics MathCore Thread MultiProc ROOTDataFrame m dl Thread XMLParser XMLIO RHTTP XMLParser XMLIO RHTTP)
link_libraries(${ROOTANALIBS})
#This below is broken and needs a REGEX wizard:
#Get the libs for rootana (from rootana/include/rootana_libs.txt )
#file( READ $ENV{ROOTANASYS}/include/rootana_libs.txt rootana_libs) 
#string(REPLACE "\n" "" rootana_libs "${rootana_libs}" )
#list(APPEND ROOTANALIBS ${rootana_libs})
#link_libraries(rootana ${ROOTANALIBS})

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

link_directories($ENV{ROOTANASYS}/lib)

# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# MIDAS -- live analyzer
set(MIDASSYS "" CACHE PATH "")
if(MIDASSYS STREQUAL "")
    set(MIDASSYS "$ENV{MIDASSYS}")
endif()
if(MIDASSYS STREQUAL "")
    find_package(Midas QUIET)
else()
    #MIDAS pre cmake compatability
    set(MIDAS_FOUND TRUE)
    set(MIDAS_INCLUDE_DIRS "$ENV{MIDASSYS}/include")
    set(MIDAS_LIBRARIES "$ENV{MIDASSYS}/lib/libmidas.a" -lutil -lrt)
endif()
if(MIDAS_FOUND)
  message(STATUS "MIDAS found")
  message(DEBUG "[${PROJECT_NAME}] MIDAS_INCLUDE_DIRS = ${MIDAS_INCLUDE_DIRS}")
  message(DEBUG "[${PROJECT_NAME}] MIDAS_LIBRARIES = ${MIDAS_LIBRARIES}")
  link_libraries(${MIDAS_LIBRARIES})
  link_directories($ENV{MIDASSYS}/lib)
endif()

# Include directories
include_directories(include)

#Analysis Report needs A2Flow in alpha2...
#include_directories(../alpha2/include)
include_directories(${CMAKE_SOURCE_DIR}/alpha2/include)
#...which then needs UnpackVF48 in a2lib
#include_directories(../a2lib/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)


#AG compatability includes
include_directories(${CMAKE_SOURCE_DIR}/analib/include)
include_directories(${CMAKE_SOURCE_DIR}/ana/include)
include_directories(${ROOT_INCLUDE_DIRS})

if (BUILT_IN_ROOTANA)
   include_directories(include ../rootana/include)
else ()
   include_directories(include $ENV{ROOTANASYS}/include)
endif (BUILT_IN_ROOTANA)

#feGEM (labVIEW logging)
include_directories($ENV{AGRELEASE}/feGEM)

message(${ROOT_INCLUDE_DIRS})

#Find all *.modules files
file(GLOB BINS *.modules)

message(Binaries: ${BINS})
foreach( LIST_FILE ${BINS} )
    string( REPLACE ".modules" ".exe" bin ${LIST_FILE} )
    get_filename_component(bin ${bin} NAME)
    message(Binary: ${bin})
    
    message(List file: ${LIST_FILE})
    execute_process( COMMAND grep -v "\#" ${LIST_FILE} 
                     OUTPUT_VARIABLE MODULES )
    #Clean up any strange formatting choices
    string( REPLACE "\n" " " MODULES ${MODULES} )
    string( REPLACE "\t" " " MODULES ${MODULES} )
    #.o files come from .cxx files... cmake wants this
    string( REPLACE ".o" ".cxx" MODULES ${MODULES} )
    #Turn string into list
    separate_arguments(MODULES)
    #Prepend path to source files
    list(TRANSFORM MODULES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/src/)
    #message(Modules: ${MODULES})
    
    add_executable( ${bin} $ENV{ROOTANASYS}/manalyzer/manalyzer_main  ${MODULES} )
    
    # Make sure YourLib is linked to each app
    target_include_directories(${bin} PUBLIC include)
    target_include_directories(${bin} PUBLIC ../analib/include)
    if (BUILD_AG)
       target_include_directories(${bin} PUBLIC ../recolib/include)
    endif(BUILD_AG)
    target_include_directories(${bin} PUBLIC ${ROOT_INCLUDE_DIRS})
    link_directories(../alpha2 ../analib )
    find_library(LIBXM Xm)
    find_library(LIBXT Xt)
    find_library(LIBSQLITE3 sqlite3)
    target_link_libraries( ${bin} ${ROOT_LIBRARIES} ${LIBXT} ${LIBXM} ${LIBSQLITE3} alpha2 analib rootana ${ROOTANALIBS} )
    install(TARGETS ${bin} DESTINATION "${CMAKE_INSTALL_PREFIX}")
endforeach( LIST_FILE ${BINS} )
