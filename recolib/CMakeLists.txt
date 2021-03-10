# CMakeLists.txt for basic analysis package.
# It creates a library with dictionary 
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agtpc)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/agana)
include_directories(${CMAKE_SOURCE_DIR}/analib/include)
include_directories(${CMAKE_SOURCE_DIR}/ana/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
#include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)
if (BUILT_IN_ROOTANA)
   include_directories(${CMAKE_SOURCE_DIR}/rootana/include)
else ()
   include_directories($ENV{ROOTANASYS}/include)
endif (BUILT_IN_ROOTANA)

if (BUILD_AG_SIM)
  include_directories(${CMAKE_SOURCE_DIR}/simulation/G4out/include)
endif (BUILD_AG_SIM)
include_directories(${PROJECT_SOURCE_DIR}/include)

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.hh)
ROOT_GENERATE_DICTIONARY(G__agtpc ${LINK_HEADER_FILES} LINKDEF RecoLinkDef.hh)

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx)
add_library(agtpc SHARED ${LIBRARY_SOURCES} G__agtpc)
target_include_directories(agtpc PUBLIC include)
#target_link_libraries(agtpc agana ${ROOT_LIBRARIES})

install(TARGETS agtpc DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libagtpc_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libagtpc.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.hh)
install(FILES ${HEADER_FILES} DESTINATION include)
