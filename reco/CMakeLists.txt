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


# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
if(EXISTS $ENV{ROOTSYS}/ROOTConfig.cmake)
    if(NOT DEFINED ENV{CMAKE_PREFIX_PATH})
        list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
    endif()
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/etc/cmake")

include_directories(${ROOT_INCLUDE_DIRS})

# Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Minuit2 MathMore Spectrum )
link_libraries(${ROOT_LIBRARIES} XMLParser XMLIO XMLParser XMLIO)


# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories($ENV{AGRELEASE}/recolib/include)
include_directories($ENV{AGRELEASE}/analib/include)
include_directories($ENV{AGRELEASE}/ana/include)
include_directories($ENV{ROOTANASYS}/include) # manalyzer required in Match?
include_directories($ENV{AGRELEASE}/simulation/geant4/include)

include(${ROOT_USE_FILE})
ROOT_GENERATE_DICTIONARY(G4inDict $ENV{AGRELEASE}/simulation/geant4/include/TWaveform.hh $ENV{AGRELEASE}/simulation/geant4/include/TMChit.hh LINKDEF $ENV{AGRELEASE}/simulation/geant4/include/LinkDef.hh)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

link_libraries(AGTPC agana)

add_executable( agreco.exe agreco.cc ${sources} ${headers} )
add_executable( g4ana.exe g4ana.cc G4inDict.cxx ${sources} ${headers} )

#link_directories( $ENV{AGRELEASE}/build/analib $ENV{AGRELEASE}/build/recolib )
#target_link_libraries(g4ana.exe AGTPC ${ROOT_LIBRARIES} Minuit2 )

    
install(TARGETS agreco.exe g4ana.exe DESTINATION "${CMAKE_INSTALL_PREFIX}")
