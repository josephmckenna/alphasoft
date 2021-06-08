#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.3)
project(gpp)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

#----------------------------------------------------------------------------
# Find ROOT (required package)
#
find_package(ROOT 6.0 REQUIRED COMPONENTS Geom Gdml)
#--- Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})
include_directories(${ROOT_INCLUDE_DIR})

#----------------------------------------------------------------------------
# Garfield++
#
find_package(Garfield REQUIRED)
include_directories(${GARFIELD_INCLUDE_DIRS})
link_libraries(${GARFIELD_LIBRARIES})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/ComponentBmap.cc ${PROJECT_SOURCE_DIR}/src/MagneticFieldMap.cc ${PROJECT_SOURCE_DIR}/src/TPCBase.cc ${PROJECT_SOURCE_DIR}/src/TPC.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

set(BINS DriftGas.cc PlotMedium.cc ChamberField.cc CalculateGain.cc ChamberDrift.cc ElectronAvalanche.cc TrackElectronAvalanche.cc WireSag.cc polar.cc PolarDrift.cc PolarSignal.cc PolarDriftSignal.cc)

foreach(target ${BINS})
  string( REPLACE ".cc" ".exe" bin ${target} )
  add_executable(${bin} ${PROJECT_SOURCE_DIR}/src/${target} ${sources})
  #target_link_libraries(${bin} ${ROOT_LIBRARIES}  Garfield::magboltz Garfield::Garfield )
  target_link_libraries(${bin} ${ROOT_LIBRARIES} Garfield )
  install(TARGETS ${bin} DESTINATION simulation)
endforeach(target ${BINS})
