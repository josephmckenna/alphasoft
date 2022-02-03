#----------------------------------------------------------------------------
# Setup the project
#
cmake_minimum_required(VERSION 3.3)
project(gpp)

set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_BUILD_TYPE RelWithDebInfo)

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

find_path(Garfield_INCLUDE_DIRS Sensor.hh
        HINTS $ENV{GARFIELD_INSTALL}/include
        PATH_SUFFIXES Garfield)

include_directories(${Garfield_INCLUDE_DIRS} ${Garfield_INCLUDE_DIRS}/.. ${GARFIELD_INCLUDE_DIRS})

find_library(Garfield_LIBRARIES libGarfield.so
        HINTS $ENV{GARFIELD_INSTALL}/lib $ENV{GARFIELD_INSTALL}/lib64)

link_libraries(${Garfield_LIBRARIES} ${GARFIELD_LIBRARIES})

# get_cmake_property(_variableNames VARIABLES)
# list (SORT _variableNames)
# foreach (_variableName ${_variableNames})
#     message(STATUS "${_variableName}=${${_variableName}}")
# endforeach()

message(STATUS "Garfieldpp includes: ${Garfield_INCLUDE_DIRS} ${GARFIELD_INCLUDE_DIRS} libs: ${Garfield_LIBRARIES} ${GARFIELD_LIBRARIES}")

execute_process(COMMAND grep -c EnableMagneticField ${Garfield_INCLUDE_DIRS}/AvalancheMC.hh OUTPUT_VARIABLE FOUND_EN_MAG)

if(${FOUND_EN_MAG} GREATER 0)
message(STATUS "Old version of Garfield++, consider updating")
add_compile_definitions(OLD_AV_MC)
endif()

#----------------------------------------------------------------------------
# Locate sources and headers for this project
#
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB sources ${PROJECT_SOURCE_DIR}/src/ComponentBmap.cc ${PROJECT_SOURCE_DIR}/src/MagneticFieldMap.cc ${PROJECT_SOURCE_DIR}/src/TPCBase.cc ${PROJECT_SOURCE_DIR}/src/TPC.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

set(OLDBINS ChamberField.cc CalculateGain.cc ChamberDrift.cc ElectronAvalanche.cc TrackElectronAvalanche.cc WireSag.cc)
set(NEWBINS DriftGas.cc PlotMedium.cc polar.cc PolarDrift.cc PolarSignal.cc PolarDriftSignal.cc)

foreach(target ${OLDBINS})
  string( REPLACE ".cc" ".exe" bin ${target} )
  add_executable(${bin} ${PROJECT_SOURCE_DIR}/src/${target} ${sources})
  target_link_libraries(${bin} ${ROOT_LIBRARIES} ${GARFIELD_LIBRARIES})
  install(TARGETS ${bin} DESTINATION simulation)
endforeach(target ${OLDBINS})

foreach(target ${NEWBINS})
  string( REPLACE ".cc" ".exe" bin ${target} )
  add_executable(${bin} ${PROJECT_SOURCE_DIR}/src/${target})
  target_link_libraries(${bin} ${ROOT_LIBRARIES} ${GARFIELD_LIBRARIES})
  install(TARGETS ${bin} DESTINATION simulation)
endforeach(target ${NEWBINS})
