#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(AgTPC)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
#CADMesh 
find_package(cadmesh REQUIRED)
if(cadmesh_FOUND)
  message(STATUS "CADMesh found!")
  #message(STATUS "${CADMESH_INCLUDE_DIRS}")
endif(cadmesh_FOUND)
# just write it down explicitly down the 
#set(CADMESH_INCLUDE_DIRS $ENV{CADMESH_HOME}/include)
#set(cadmesh_LIBRARIES -L$ENV{CADMESH_HOME}/lib cadmesh assimp tet)
#

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT QUIET REQUIRED)
if(NOT ROOT_FOUND)
  message(STATUS "AgTPC: ROOT package not found.")  
  return()
endif()
include(${ROOT_USE_FILE})
EXECUTE_PROCESS(COMMAND 
  rootcling -f ${PROJECT_BINARY_DIR}/dict.cc -c ${PROJECT_SOURCE_DIR}/include/TMChit.hh ${PROJECT_SOURCE_DIR}/include/TWaveform.hh ${PROJECT_SOURCE_DIR}/include/LinkDef.hh)
EXECUTE_PROCESS(COMMAND 
  root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-shadow ${ROOT_CXX_FLAGS}")

#----------------------------------------------------------------------------
# include CRY (required package)
#
set(CRY $ENV{CRYHOME})
set(CRY_INCLUDE_DIR ${CRY}/src)
set(CRY_LIBRARIES ${CRY}/lib/libCRY.a)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
set(GARFIELDPP $ENV{AGRELEASE}/simulation/garfieldpp)
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${GARFIELDPP}/include
		    ${Geant4_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIR}
		    $ENV{GARFIELD_HOME}/Include
		    $ENV{GARFIELD_HOME}/Heed
		    ${CRY_INCLUDE_DIR}
		    ${CADMESH_INCLUDE_DIRS})
link_directories($ENV{GARFIELD_HOME}/Library)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc ${PROJECT_BINARY_DIR}/dict.cc ${GARFIELDPP}/src/ComponentBmap.cc ${GARFIELDPP}/src/MagneticFieldMap.cc ${GARFIELDPP}/src/TPCBase.cc ${GARFIELDPP}/src/TPC.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)
#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(AGTPC AGTPC.cc ${sources} ${headers})
EXECUTE_PROCESS(COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_EXE_LINKER_FLAGS "${ROOT_LD_FLAGS}")

target_link_libraries(AGTPC ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} -lGarfield -lgfortran -lGeom ${CRY_LIBRARIES} ${cadmesh_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build AGTPC. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(AGTPC_SCRIPTS
  cry.file vis.mac runHeedInterface.mac runHeedOnly.mac
  )

foreach(_script ${AGTPC_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()
#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS AGTPC DESTINATION bin)

