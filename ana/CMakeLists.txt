# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alphasoft)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)


# Version numbering
set (basicApp_VERSION_MAJOR 1)
set (basicApp_VERSION_MAJOR 0)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

macro(replace_list_item LIST OLD_VALUE NEW_VALUE)
    list(FIND ${LIST} ${OLD_VALUE} OLD_VALUE_INDEX)
    if(OLD_VALUE_INDEX GREATER_EQUAL 0)
        list(REMOVE_AT ${LIST} ${OLD_VALUE_INDEX})
        list(INSERT ${LIST} ${OLD_VALUE_INDEX} ${NEW_VALUE})
    endif()
endmacro()


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
#Extra path for GitInfo.h file
include_directories(${CMAKE_BINARY_DIR})

find_package(ROOTANA)
include_directories(include ${ROOTANA_INCLUDE_DIRS})
message(STATUS "ROOTANA headers: ${ROOTANA_INCLUDE_DIRS}")

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
find_package(ROOT REQUIRED COMPONENTS MathMore Minuit2 XMLParser Spectrum)
include_directories(${ROOT_INCLUDE_DIRS})
link_libraries(${ROOT_LIBRARIES})
# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
message(STATUS "ROOT headers: ${ROOT_INCLUDE_DIRS}")



#find_package(X11 REQUIRED)
#link_libraries(${X11_LIBRARIES})
#include_directories(${X11_INCLUDE_DIR})


# MIDAS -- live analyzer
set(MIDASSYS "" CACHE PATH "")
if(MIDASSYS STREQUAL "")
    set(MIDASSYS "$ENV{MIDASSYS}")
endif()
find_package(Midas QUIET)
if(MIDAS_FOUND)
  set(MIDASSYS "$ENV{MIDASSYS}")
  message(STATUS "MIDAS found ${MIDASSYS}")
  message(DEBUG "[${PROJECT_NAME}] MIDAS_INCLUDE_DIRS = ${MIDAS_INCLUDE_DIRS}")
  message(DEBUG "[${PROJECT_NAME}] MIDAS_LIBRARIES = ${MIDAS_LIBRARIES}")
  link_libraries(${MIDAS_LIBRARIES})
  link_directories(${MIDAS_LIBRARY_DIRS})
endif()


include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)
include_directories(${CMAKE_SOURCE_DIR}/alpha2/include)
link_directories(${CMAKE_SOURCE_DIR}/analib)
find_library(LIBXM Xm)
find_library(LIBXT Xt)

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

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/unpack_module.cxx" 
        "../agana/unpack_module.cxx"
    )

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pwb_module.cxx" 
        "../agana/pwb_module.cxx"
    )

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/adc_module.cxx" 
        "../agana/adc_module.cxx"
    )

    #message(Modules: ${MODULES})
    if (BUILT_IN_ROOTANA)
       add_executable( ${bin} ../rootana/manalyzer/manalyzer_main  ${MODULES} )
    else ()
       add_executable( ${bin} $ENV{ROOTANASYS}/manalyzer/manalyzer_main  ${MODULES} )
    endif (BUILT_IN_ROOTANA)
    
    
    #add_dependencies(${bin} gitinfo)
    # Make sure YourLib is linked to each app
    target_include_directories(${bin} PUBLIC include)
    target_include_directories(${bin} PUBLIC ${ROOT_INCLUDE_DIRS})
    link_directories(${bin} analib)

    find_library(LIBXM Xm)
    find_library(LIBXT Xt)
    target_link_libraries( ${bin} ${ROOT_LIBRARIES} ${LIBXT} ${LIBXM} Minuit2 agana analib agtpc aged rootana ${ROOTANALIBS} ) 
    
    install(TARGETS ${bin}  DESTINATION "${CMAKE_INSTALL_PREFIX}" )
endforeach( LIST_FILE ${BINS} )
