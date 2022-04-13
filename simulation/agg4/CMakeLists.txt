#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agg4)

#----------------------------------------------------------------------------
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_PREFIX}/lib)
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
  message(STATUS "agg4: ROOT package not found.")  
  return()
else()
  # useful macros such as ROOT_GENERATE_DICTIONARY below
  include(${ROOT_USE_FILE})
endif()
include_directories(${ROOT_INCLUDE_DIR})

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

ROOT_GENERATE_DICTIONARY( G__G4dict TDigi.hh TScintDigi.hh TBSCHit.hh TTPCelement.hh TAnode.hh TPads.hh LINKDEF include/G4dictLinkDef.hh )
set(LIBRARY_SOURCES ${PROJECT_SOURCE_DIR}/src/TDigi.cc 
                    ${PROJECT_SOURCE_DIR}/src/TScintDigi.cc 
                    ${PROJECT_SOURCE_DIR}/src/TBSCHit.cc 
                    ${PROJECT_SOURCE_DIR}/src/TTPCelement.cc 
                    ${PROJECT_SOURCE_DIR}/src/TAnode.cc 
                    ${PROJECT_SOURCE_DIR}/src/TPads.cc
                    ${PROJECT_SOURCE_DIR}/src/ElectronDrift.cc
                    ${PROJECT_SOURCE_DIR}/../garfieldpp/src/TPCBase.cc
                    ${PROJECT_SOURCE_DIR}/src/agg4globals.cc)
add_library(G4dict SHARED ${LIBRARY_SOURCES} G__G4dict)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
list(APPEND sources ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/src/TPCBase.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
add_executable(agg4 agg4.cc G__G4dict ${sources} ${headers})
add_compile_definitions(CAD_FILE_PATH="${AG_CAD_FILE_PATH}")
add_compile_definitions(CRYDATAPATH="${CRY_DATA_PATH}")
target_link_libraries(agg4 ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${CRY_LIBRARIES} cry cadmesh G4out G4dict)

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build agg4. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(AGG4_SCRIPTS
  cosmics.mac pionly.mac run.mac test.mac)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS agg4 DESTINATION simulation)
install(TARGETS G4dict DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libG4dict_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libG4dict.rootmap DESTINATION lib)
install(FILES ${headers} DESTINATION include)
install(FILES ${AGG4_SCRIPTS} DESTINATION simulation)
