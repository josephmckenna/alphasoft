# CMakeLists.txt for basic analysis package.
# It creates 3 main programs 
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agreco)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Include directories
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${ROOTANA_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/rootUtils/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
include_directories(${CMAKE_SOURCE_DIR}/analib/include)
include_directories(${CMAKE_SOURCE_DIR}/simulation/G4out/include)
include_directories(include)

# Find source for this project
set(sources ${PROJECT_SOURCE_DIR}/src/Histo.cc ${PROJECT_SOURCE_DIR}/src/Utils.cc)
set(headers ${PROJECT_SOURCE_DIR}/include/Histo.hh ${PROJECT_SOURCE_DIR}/include/Utils.hh 
            ${PROJECT_SOURCE_DIR}/include/argparse.hh)

# Create the targets
add_executable( agreco.exe agreco.cc ${sources} ${headers} )
target_link_libraries(agreco.exe ${ROOT_LIBRARIES} analib agtpc)

add_executable( MainEventTree.exe MainEventTree.cc 
                  ${sources} ${PROJECT_SOURCE_DIR}/src/ReadEventTree.cc
                  ${headers} ${PROJECT_SOURCE_DIR}/include/ReadEventTree.hh)
target_link_libraries(MainEventTree.exe ${ROOT_LIBRARIES} analib agtpc alpha2 rootUtils)
    
# Install 
install(TARGETS agreco.exe MainEventTree.exe DESTINATION ${CMAKE_INSTALL_PREFIX})

# Compile the analyzer for simulation
if (BUILD_AG_SIM)
  message(STATUS "Building Geant4 analyzer")
  add_executable( g4ana.exe g4ana.cc 
                    ${sources} ${PROJECT_SOURCE_DIR}/src/ProcessEvents.cc
                    ${headers} ${PROJECT_SOURCE_DIR}/include/ProcessEvents.hh 
		    ${PROJECT_SOURCE_DIR}/include/fileutility.hh )
  target_include_directories(g4ana.exe PUBLIC ${CMAKE_SOURCE_DIR}/agana)
  target_link_libraries(g4ana.exe ${ROOT_LIBRARIES} agtpc G4out)
  install(TARGETS g4ana.exe DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(BUILD_AG_SIM)
