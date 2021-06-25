#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(rTPCsim)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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
include_directories(${Geant4_INCLUDE_DIR})

#----------------------------------------------------------------------------
# CADMesh
#
include_directories(${CMAKE_SOURCE_DIR}/simulation/CADMesh/include)
include_directories(${CMAKE_BINARY_DIR}/simulation/CADMesh/include)
link_directories(${CMAKE_BINARY_DIR}/simulation/CADMesh)


#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT REQUIRED COMPONENTS MathMore)
if(NOT ROOT_FOUND)
  message(STATUS "rTPCsim: ROOT package not found.")  
  return()
else()
  # useful macros such as ROOT_GENERATE_DICTIONARY below
  include(${ROOT_USE_FILE})
endif()
include_directories(${ROOT_INCLUDE_DIR})

#----------------------------------------------------------------------------
# Garfield++
#
find_package( Garfield REQUIRED)

#----------------------------------------------------------------------------
# include CRY (required package)
#
include_directories(${CMAKE_SOURCE_DIR}/simulation/cry/include)
link_directories(${CMAKE_BINARY_DIR}/simulation/cry)

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/include
		    ${CMAKE_SOURCE_DIR}/simulation/G4out/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
list(APPEND sources ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/src/ComponentBmap.cc ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/src/MagneticFieldMap.cc ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/src/TPCBase.cc ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/src/TPC.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4, 
# Garfield++, CRY, CADMesh and ROOT libraries
#
add_executable(rTPCsim rTPCsim.cc ${sources} ${headers})
add_compile_definitions(CAD_FILE_PATH="${AG_CAD_FILE_PATH}")
add_compile_definitions(CRYDATAPATH="${CRY_DATA_PATH}")
target_link_libraries(rTPCsim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${GARFIELD_LIBRARIES} ${CRY_LIBRARIES} cry cadmesh G4out)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build rTPCsim. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(SIM_SCRIPTS
  cry.file init_vis.mac vis.mac runHeedInterface.mac runHeedInterfaceAvalMC.mac runHeedOnly.mac
  )


#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS rTPCsim DESTINATION simulation)
file(GLOB HEADER_FILES include/*.hh)
install(FILES ${HEADER_FILES} DESTINATION include)
install(FILES ${SIM_SCRIPTS} DESTINATION simulation)

