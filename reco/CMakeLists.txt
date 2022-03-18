# CMakeLists.txt for basic analysis package.
# It creates 3 main programs 
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agreco)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

# Include directories
include_directories(${ROOT_INCLUDE_DIRS})
include_directories(${CMAKE_SOURCE_DIR}/rootUtils/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)
include_directories(${CMAKE_SOURCE_DIR}/libGEM/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
include_directories(${CMAKE_SOURCE_DIR}/analib/include)
include_directories(${CMAKE_SOURCE_DIR}/ana/include)
include_directories(${CMAKE_SOURCE_DIR}/simulation/G4out/include)
include_directories(include)

# Find source for this project
set(sources ${PROJECT_SOURCE_DIR}/src/Histo.cc ${PROJECT_SOURCE_DIR}/src/Utils.cc)
set(headers ${PROJECT_SOURCE_DIR}/include/Histo.hh ${PROJECT_SOURCE_DIR}/include/Utils.hh 
            ${PROJECT_SOURCE_DIR}/include/argparse.hh)

# Libraries needed
set(AGRECO_LINK_LIBS agtpc analib)
if(BUILD_A2)
  set(AGTREE_LINK_LIBS analib alpha2 rootUtils)
else()
  set(AGTREE_LINK_LIBS agtpc analib rootUtils)
endif()

# Create the targets
add_executable( agreco.exe agreco.cc ${sources} ${headers} )
add_dependencies(agreco.exe analib agtpc)
target_link_libraries(agreco.exe ${ROOT_LIBRARIES} ${AGRECO_LINK_LIBS})

add_executable( MainEventTree.exe MainEventTree.cc 
                ${sources} ${PROJECT_SOURCE_DIR}/src/ReadEventTree.cc
                ${headers} ${PROJECT_SOURCE_DIR}/include/ReadEventTree.hh)
if(BUILD_A2)
  add_dependencies(MainEventTree.exe analib agtpc alpha2 rootUtils)
else()
  add_dependencies(MainEventTree.exe analib agtpc rootUtils)
endif()
target_link_libraries(MainEventTree.exe ${ROOT_LIBRARIES} ${AGTREE_LINK_LIBS})

    
# Install 
install(TARGETS agreco.exe MainEventTree.exe DESTINATION ${CMAKE_INSTALL_PREFIX})

# Compile the analyzer for simulation
if (BUILD_AG_SIM)
  message(STATUS "Building Geant4 analyzer")
  set(G4ANA_LINK_LIBS agtpc G4out)
  add_executable( g4ana.exe g4ana.cc 
                    ${sources} ${PROJECT_SOURCE_DIR}/src/ProcessEvents.cc
                    ${headers} ${PROJECT_SOURCE_DIR}/include/ProcessEvents.hh 
		    ${PROJECT_SOURCE_DIR}/include/fileutility.hh )
  add_dependencies(g4ana.exe agtpc G4out)
  target_include_directories(g4ana.exe PUBLIC ${CMAKE_SOURCE_DIR}/agana)
  target_link_libraries(g4ana.exe ${ROOT_LIBRARIES} ${G4ANA_LINK_LIBS})
  install(TARGETS g4ana.exe DESTINATION ${CMAKE_INSTALL_PREFIX})

  message(STATUS "Building (Geant4) BSC analyzer")
  add_executable( baranalysis.exe baranalysis.cc 
    ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/src/TPCBase.cc
    ${CMAKE_SOURCE_DIR}/simulation/agg4/src/ElectronDrift.cc )
  add_dependencies(baranalysis.exe agtpc G4dict)
  target_include_directories(baranalysis.exe PUBLIC ${CMAKE_SOURCE_DIR}/simulation/agg4/include ${CMAKE_SOURCE_DIR}/simulation/garfieldpp/include)
  target_link_libraries(baranalysis.exe ${ROOT_LIBRARIES} agtpc G4dict)
  install(TARGETS baranalysis.exe DESTINATION ${CMAKE_INSTALL_PREFIX})
endif(BUILD_AG_SIM)
