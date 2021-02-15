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
find_package(ROOT REQUIRED COMPONENTS MathMore)
if(NOT ROOT_FOUND)
  message(STATUS "Geant4: ROOT package not found.")  
  return()
else()
  # useful macros such as ROOT_GENERATE_DICTIONARY below
  include(${ROOT_USE_FILE})
endif()

#----------------------------------------------------------------------------
# include CRY (required package)
#
ExternalProject_Get_Property(CRY-1.7 source_dir)
set(CRY_INCLUDE_DIRS "${source_dir}/src")
set(CRY_LIBRARY_DIR "${source_dir}/lib")
set(CRY_LIBRARIES "libCRY.a")
link_directories(${CRY_LIBRARY_DIR})
include_directories(${CRY_INCLUDE_DIRS})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include 
                    ${Geant4_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIR}
		    ${CRY_INCLUDE_DIR}
		    ${CADMESH_INCLUDE_DIRS}
		    $ENV{AGRELEASE}/simulation/garfieldpp/include)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# 
#ROOT_GENERATE_DICTIONARY( G4dict include/TMChit.hh include/TDigi.hh include/TScintDigi.hh include/TTPCelement.hh include/TAnode.hh include/TPads.hh include/TWaveform.hh LINKDEF include/G4outLinkDef.hh )
ROOT_GENERATE_DICTIONARY( G__G4dict TMChit.hh TDigi.hh TScintDigi.hh TTPCelement.hh TAnode.hh TPads.hh TWaveform.hh LINKDEF include/G4outLinkDef.hh )


#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(agg4 agg4.cc $ENV{AGRELEASE}/simulation/garfieldpp/src/TPCBase.cc ${sources} ${headers} G__G4dict.cxx)
target_link_libraries(agg4 ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${CRY_LIBRARIES} ${cadmesh_LIBRARIES}) # ${AG_LIBRARIES}

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build agg4. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(AGG4_SCRIPTS
  settings.dat annihilation.dat cry.file cosmics.mac pionly.mac ResolutionTestsOnAxis.mac ResolutionTestsOnGas.mac run.mac vis.mac multirun.sh
  )

##----------------------------------------------------------------------------
## Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
##
install(TARGETS agg4 DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${AGG4_SCRIPTS} DESTINATION ${CMAKE_INSTALL_PREFIX}/agg4_scripts)
# I want these in ${CMAKE_INSTALL_PREFIX}/lib I think... but it gets upset... I guess because we are dealing with an .exe and not a .so?
install(FILES ${PROJECT_BINARY_DIR}/libG4dict_rdict.pcm DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${PROJECT_BINARY_DIR}/libG4dict.rootmap DESTINATION ${CMAKE_INSTALL_PREFIX})