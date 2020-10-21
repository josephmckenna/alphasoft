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
#Extra path for gitinfo.h file
include_directories(${CMAKE_CURRENT_BINARY_DIR})

add_custom_target(
    a2gitinfo ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/__header.h
    )

add_custom_command(
    OUTPUT
        ${CMAKE_CURRENT_BINARY_DIR}/__header.h  # fake! ensure we run!
        ${CMAKE_CURRENT_BINARY_DIR}/GitInfo.h    # real header, we write.
    # this command must generate: ${CMAKE_CURRENT_BINARY_DIR}/header.h
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/GitInfo.sh ${CMAKE_CURRENT_BINARY_DIR}
    )

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

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

#link_libraries(agana AGTPC aged)
link_directories($ENV{ROOTANASYS}/lib)

# analib/ AGTPC aged)
#link_directories(analib/ recolib/ aged/)
# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# Include directories
include_directories(include)

#Analysis Report needs A2Flow in alpha2...
include_directories($ENV{AGRELEASE}/alpha2/include)
#...which then needs UnpackVF48 in a2lib
include_directories($ENV{AGRELEASE}/a2lib/include)

#AG compatability includes
include_directories($ENV{AGRELEASE}/recolib/include)
include_directories($ENV{AGRELEASE}/analib/include)
include_directories($ENV{AGRELEASE}/aged/include)
include_directories($ENV{AGRELEASE}/ana/include)
include_directories(${ROOT_INCLUDE_DIRS})

include_directories(include $ENV{ROOTANASYS}/include)


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
    
    add_dependencies(${bin} a2gitinfo)
    # Make sure YourLib is linked to each app
    target_include_directories(${bin} PUBLIC include)
    target_include_directories(${bin} PUBLIC $ENV{AGRELEASE}/recolib/include)
    target_include_directories(${bin} PUBLIC $ENV{AGRELEASE}/analib/include)
    target_include_directories(${bin} PUBLIC $ENV{AGRELEASE}/aged/include)
    target_include_directories(${bin} PUBLIC ${ROOT_INCLUDE_DIRS})
    link_directories($ENV{AGRELEASE}/alpha2 $ENV{AGRELEASE}/analib $ENV{AGRELEASE}/recolib)
    find_library(LIBXM Xm)
    find_library(LIBXT Xt)
    find_library(LIBSQLITE3 sqlite3)
    target_link_libraries( ${bin} ${ROOT_LIBRARIES} ${LIBXT} ${LIBXM} ${LIBSQLITE3} alpha2 rootana analib agtpc )
    install(TARGETS ${bin} DESTINATION "${CMAKE_INSTALL_PREFIX}")
endforeach( LIST_FILE ${BINS} )
