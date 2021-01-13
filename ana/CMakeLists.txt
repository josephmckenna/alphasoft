# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agdaq)

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

find_package(ROOTANA)
include_directories(include ${ROOTANA_INCLUDE_DIRS})

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
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/cmake")
message(${CMAKE_MODULE_PATH})

# Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Minuit2 XMLParser Spectrum)
link_libraries(${ROOT_LIBRARIES})

find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

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
include_directories($ENV{AGRELEASE}/agana)
include_directories(include)
include_directories($ENV{AGRELEASE}/recolib/include)
include_directories($ENV{AGRELEASE}/analib/include)
include_directories($ENV{AGRELEASE}/aged/include)
include_directories(${ROOT_INCLUDE_DIRS})

link_directories(${bin} $ENV{AGRELEASE}/analib)
find_library(LIBXM Xm)
find_library(LIBXT Xt)


#Analysis Report needs A2Flow in alpha2...
include_directories($ENV{AGRELEASE}/alpha2/include)
#...which then needs UnpackVF48 in a2lib
include_directories($ENV{AGRELEASE}/a2lib/include)
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

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/unpack_module.cxx" 
        "$ENV{AGRELEASE}/agana/unpack_module.cxx"
    )

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pwb_module.cxx" 
        "$ENV{AGRELEASE}/agana/pwb_module.cxx"
    )

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/adc_module.cxx" 
        "$ENV{AGRELEASE}/agana/adc_module.cxx"
    )

    #message(Modules: ${MODULES})
    
    add_executable( ${bin} $ENV{ROOTANASYS}/manalyzer/manalyzer_main  ${MODULES} )
    
    #add_dependencies(${bin} gitinfo)
    # Make sure YourLib is linked to each app
    target_include_directories(${bin} PUBLIC include)
    target_include_directories(${bin} PUBLIC agana)
    target_include_directories(${bin} PUBLIC $ENV{AGRELEASE}/recolib/include)
    target_include_directories(${bin} PUBLIC $ENV{AGRELEASE}/analib/include)
    target_include_directories(${bin} PUBLIC $ENV{AGRELEASE}/aged/include)
    target_include_directories(${bin} PUBLIC ${ROOT_INCLUDE_DIRS})
    link_directories(${bin} analib)

    find_library(LIBXM Xm)
    find_library(LIBXT Xt)
    target_link_libraries( ${bin} ${ROOT_LIBRARIES} ${LIBXT} ${LIBXM} Minuit2 agana analib agtpc aged rootana ${ROOTANALIBS} ) 
    
    install(TARGETS ${bin} DESTINATION "${CMAKE_INSTALL_PREFIX}")
endforeach( LIST_FILE ${BINS} )
