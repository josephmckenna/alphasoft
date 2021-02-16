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
link_libraries(${ROOT_LIBRARIES} XMLParser XMLIO XMLParser XMLIO Minuit2)

# Include directories
include_directories(../agana)
include_directories(../rootUtils/include)
include_directories(../a2lib/include)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(../recolib/include)
include_directories(../analib)
include_directories(../analib/include)
include_directories(../ana/include)
include_directories(../simulation/geant4/include)


find_package(ROOTANA)
message(${ROOTANA_INCLUDE_DIRS})
include_directories(include ${ROOTANA_INCLUDE_DIRS})


include(${ROOT_USE_FILE})
ROOT_GENERATE_DICTIONARY(G4inDict ../simulation/geant4/include/TWaveform.hh ../simulation/geant4/include/TMChit.hh LINKDEF ../simulation/geant4/include/G4outLinkDef.hh)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

link_libraries(agana)
if (BUILD_AG)
   link_libraries(agtpc analib )
endif (BUILD_AG)
if (BUILD_A2)
   link_libraries(agtpc alpha2 )
endif (BUILD_A2)
link_libraries(rootUtils Geom ${ROOT_LIBRARIES})
 
add_executable( agreco.exe agreco.cc ${sources} ${headers} )
add_executable( g4ana.exe g4ana.cc G4inDict.cxx ${sources} ${headers} )
add_executable( MainEventTree.exe MainEventTree.cc ${sources} ${headers} )
    
#install(TARGETS agreco.exe g4ana.exe MainEventTree.exe DESTINATION "${CMAKE_INSTALL_PREFIX}")
install(TARGETS MainEventTree.exe DESTINATION "${CMAKE_INSTALL_PREFIX}")
