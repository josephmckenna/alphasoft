# CMakeLists.txt for basic analysis package.
# It creates 3 main programs 
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agreco)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Minuit2 MathMore Spectrum )
include_directories(${ROOT_INCLUDE_DIRS})

# Locate ROOTANA 
find_package(ROOTANA)
include_directories(${ROOTANA_INCLUDE_DIRS})


# Include directories
include_directories(${CMAKE_SOURCE_DIR}/agana)
include_directories(${CMAKE_SOURCE_DIR}/rootUtils/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)

include_directories(${PROJECT_SOURCE_DIR}/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
include_directories(${CMAKE_SOURCE_DIR}/analib)
include_directories(${CMAKE_SOURCE_DIR}/analib/include)
include_directories(${CMAKE_SOURCE_DIR}/ana/include)

include_directories(${CMAKE_SOURCE_DIR}/simulation/G4out/include)


file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

link_libraries(agana rootUtils)
if (BUILD_AG)
   link_libraries(agtpc analib)
endif (BUILD_AG)
if (BUILD_A2)
   link_libraries(agtpc alpha2)
endif (BUILD_A2)
 
add_executable( agreco.exe agreco.cc ${sources} ${headers} )
target_link_libraries(agreco.exe ${ROOT_LIBRARIES})
add_executable( MainEventTree.exe MainEventTree.cc ${sources} ${headers} )
target_link_libraries(MainEventTree.exe ${ROOT_LIBRARIES})
    
install(TARGETS agreco.exe MainEventTree.exe DESTINATION)# "${CMAKE_INSTALL_PREFIX}")

if (BUILD_AG_SIM)
  message(STATUS "Building Geant4 analyzer")
  add_executable( g4ana.exe g4ana.cc ${sources} ${headers} )
  target_link_libraries(g4ana.exe ${ROOT_LIBRARIES} G4out)
  install(TARGETS g4ana.exe DESTINATION)# "${CMAKE_INSTALL_PREFIX}")
endif(BUILD_AG_SIM)
