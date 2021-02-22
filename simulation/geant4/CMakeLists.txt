#----------------------------------------------------------------------------
# Setup the project
cmake_minimum_required(VERSION 2.6 FATAL_ERROR)
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

#----------------------------------------------------------------------------
#CADMesh 
find_package(cadmesh REQUIRED)
if(cadmesh_FOUND)
  #message(STATUS "${CADMESH_INCLUDE_DIRS}")
  #Its not very cmake... cadmesh's lib path and dependaces are very gnumake style :(
  add_compile_options(${cadmesh_LIBRARIES})
else()
  message(FATAL_ERROR "CADMesh not found!!!")
endif(cadmesh_FOUND)

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT QUIET REQUIRED)
if(NOT ROOT_FOUND)
  message(STATUS "rTPCsim: ROOT package not found.")  
  return()
endif()
include(${ROOT_USE_FILE})

#----------------------------------------------------------------------------
# include CRY (required package)
#
#set(CRY $ENV{CRYHOME})
#set(CRY_INCLUDE_DIR ${CRY}/src)
#set(CRY_LIBRARIES ${CRY}/lib/libCRY.a)
ExternalProject_Get_Property(CRY-1.7 source_dir)
set(CRY_INCLUDE_DIRS "${source_dir}/src")
set(CRY_LIBRARY_DIR "${source_dir}/lib")
set(CRY_LIBRARIES "libCRY.a")
link_directories(${CRY_LIBRARY_DIR})
include_directories(${CRY_INCLUDE_DIRS})
#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
set(GARFIELDPP $ENV{AGRELEASE}/simulation/garfieldpp)
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${GARFIELDPP}/include
		    ${Geant4_INCLUDE_DIR}
                    ${ROOT_INCLUDE_DIR}
        ${GARFIELD_INCLUDE_DIR}
		    ${CADMESH_INCLUDE_DIRS})
link_directories($ENV{GARFIELD_HOME}/install/lib)

ROOT_GENERATE_DICTIONARY(G__G4out TMChit.hh TWaveform.hh LINKDEF include/G4outLinkDef.hh)
set(LIBRARY_SOURCES ${PROJECT_SOURCE_DIR}/src/TMChit.cc ${PROJECT_SOURCE_DIR}/src/TWaveform.cc)
add_library(G4out SHARED ${LIBRARY_SOURCES} G__G4out.cxx)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc ${GARFIELDPP}/src/ComponentBmap.cc ${GARFIELDPP}/src/MagneticFieldMap.cc ${GARFIELDPP}/src/TPCBase.cc ${GARFIELDPP}/src/TPC.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)
#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4, 
# Garfield++, CRY, CADMesh and ROOT libraries
#
add_executable(rTPCsim rTPCsim.cc G__G4out ${sources} ${headers})
add_compile_definitions(CAD_FILE_PATH="${AG_CAD_FILE_PATH}")
target_link_libraries(rTPCsim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} -lGarfield -lgfortran -lGeom ${CRY_LIBRARIES} ${cadmesh_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build rTPCsim. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
set(SIM_SCRIPTS
  cry.file vis.mac runHeedInterface.mac runHeedOnly.mac
  )


#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
install(TARGETS rTPCsim DESTINATION simulation)
install(TARGETS G4out DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libG4out_rdict.pcm DESTINATION simulation)
install(FILES ${PROJECT_BINARY_DIR}/libG4out.rootmap DESTINATION simulation)

file(GLOB HEADER_FILES include/*.hh)
install(FILES ${HEADER_FILES} DESTINATION include)
install(FILES ${SIM_SCRIPTS} DESTINATION simulation)

