#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.3)
project(agdaq)

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT 6.0 REQUIRED COMPONENTS Geom Gdml)
#--- Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

EXECUTE_PROCESS(COMMAND 
  root-config --cflags OUTPUT_VARIABLE ROOT_CXX_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wno-shadow ${ROOT_CXX_FLAGS}")

EXECUTE_PROCESS(COMMAND root-config --libs OUTPUT_VARIABLE ROOT_LD_FLAGS OUTPUT_STRIP_TRAILING_WHITESPACE)
set(CMAKE_EXE_LINKER_FLAGS "${ROOT_LD_FLAGS}")

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include
                    ${ROOT_INCLUDE_DIR}
		    $ENV{GARFIELD_HOME}/include/Garfield
		    $ENV{GARFIELD_HOME}/Heed)
link_directories($ENV{GARFIELD_HOME}/lib $ENV{ROOTSYS}/lib)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/ComponentBmap.cc ${PROJECT_SOURCE_DIR}/src/MagneticFieldMap.cc ${PROJECT_SOURCE_DIR}/src/TPCBase.cc ${PROJECT_SOURCE_DIR}/src/TPC.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

set(BINS DriftGas.cc PlotMedium.cc ChamberField.cc CalculateGain.cc ChamberDrift.cc ElectronAvalanche.cc TrackElectronAvalanche.cc WireSag.cc polar.cc PolarDrift.cc PolarSignal.cc)

foreach(target ${BINS})

  string( REPLACE ".cc" ".exe" bin ${target} )
  add_executable(${bin} ${PROJECT_SOURCE_DIR}/src/${target} ${sources})
  target_link_libraries(${bin} ${ROOT_LIBRARIES} -lGarfield -lgfortran -lGeom)

endforeach(target ${BINS})

