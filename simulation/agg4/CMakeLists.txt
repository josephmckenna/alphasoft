#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
project(agg4)
add_definitions(-Wall -g -std=c++11)

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
find_package(cadmesh)
if(cadmesh_FOUND)
  message(STATUS "CADMesh found!")
  #message(STATUS "${CADMESH_INCLUDE_DIRS}")
endif(cadmesh_FOUND)

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT QUIET)
if(NOT ROOT_FOUND)
  message(STATUS "Geant4: ROOT package not found.")  
  return()
else()
  find_library(ROOTMATHMORE_LIBRARY MathMore
    PATHS ${ROOT_LIBRARY_DIR})
  if(ROOTMATHMORE_LIBRARY)
    list(APPEND ROOT_LIBRARIES -lMathMore)
    message(STATUS 
      "Library -lMathMore was added to the linkage list")
  else()
    message(WARNING
      "Could not find ROOT MathMore library")
  endif()
  EXECUTE_PROCESS(COMMAND 
    rootcint -f dict.cc -I$ENV{AGRELEASE}/simulation/garfieldpp/include -c ${PROJECT_SOURCE_DIR}/include/TMChit.hh ${PROJECT_SOURCE_DIR}/include/TDigi.hh ${PROJECT_SOURCE_DIR}/include/TElectronDrift.hh ${PROJECT_SOURCE_DIR}/include/TScintDigi.hh ${PROJECT_SOURCE_DIR}/include/TTPCelement.hh ${PROJECT_SOURCE_DIR}/include/TAnode.hh ${PROJECT_SOURCE_DIR}/include/TPads.hh  ${PROJECT_SOURCE_DIR}/include/TWaveform.hh ${PROJECT_SOURCE_DIR}/include/LinkDef.hh)
  EXECUTE_PROCESS(COMMAND 
    root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-shadow ${ROOT_CXX_FLAGS}")
endif()

#----------------------------------------------------------------------------
# include CRY (required package)
#
set(CRY $ENV{CRYHOME})
set(CRY_INCLUDE_DIR ${CRY}/src)
set(CRY_LIBRARIES ${CRY}/lib/libCRY.a)

#----------------------------------------------------------------------------
# include ALPHA-g DAQ and utils
#
#set(AGUTILS_INCLUDE $ENV{AGTPC_ANALYSIS} $ENV{GARFIELDPP})
#set(AG_LIBRARIES libAGUTILS.so libAGDAQ.so)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${Geant4_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIR}
		    ${CRY_INCLUDE_DIR}
		    ${CADMESH_INCLUDE_DIRS}
		    #$ENV{ANALYSIS_TPC}/include
		    #$ENV{AGANA}
		    $ENV{AGRELEASE}/simulation/garfieldpp/include)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(agg4 agg4.cc ${sources} ${headers})
if(ROOT_FOUND)
EXECUTE_PROCESS(COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_EXE_LINKER_FLAGS "${ROOT_LD_FLAGS} -I$ENV{AGRELEASE}/simulation/garfieldpp/include $ENV{AGRELEASE}/simulation/garfieldpp/src/TPCBase.cc dict.cc")
endif()
target_link_libraries(agg4 ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${CRY_LIBRARIES} ${cadmesh_LIBRARIES}) # ${AG_LIBRARIES}

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build agg4. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(AGG4_SCRIPTS
  settings.dat annihilation.dat cry.file cosmics.mac pionly.mac ResolutionTestsOnAxis.mac ResolutionTestsOnGas.mac run.mac vis.mac multirun.sh
  )

foreach(_script ${AGG4_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()
##----------------------------------------------------------------------------
## Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
##
#install(TARGETS agg4 DESTINATION bin)

