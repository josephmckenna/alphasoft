# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(analib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
include_directories(${CMAKE_SOURCE_DIR}/aglib/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.h include/*.hh)
list( REMOVE_ITEM LINK_HEADER_FILES "AnalysisFlow.h")
ROOT_GENERATE_DICTIONARY(G__analib ${LINK_HEADER_FILES} LINKDEF LibLinkDef.hh)

# Create a shared library with geneated dictionary

file(GLOB LIBRARY_SOURCES src/*.cxx)

file(GLOB AG_LIBRARY_SOURCES src/TAG*.cxx)
file(GLOB A2_LIBRARY_SOURCES src/TA2*.cxx)

# If only building A2, remove AG library sources
if (BUILD_A2 AND NOT BUILD_AG)
   list( REMOVE_ITEM LIBRARY_SOURCES ${AG_LIBRARY_SOURCES})
endif()

# If only building AG, remove AG library sources
if (BUILD_AG AND NOT BUILD_A2)
   list( REMOVE_ITEM LIBRARY_SOURCES ${A2_LIBRARY_SOURCES})
endif()



add_library(analib SHARED ${LIBRARY_SOURCES} G__analib.cxx)

target_include_directories(analib PUBLIC include)

if(BUILD_AG)
   target_link_libraries(analib PUBLIC aglib)
endif()
if(BUILD_A2)
   target_link_libraries(analib PUBLIC alpha2) # sqlite and TSISEvent
endif()
target_link_libraries(analib PUBLIC manalyzer Tree XMLParser Core RIO Hist)
add_dependencies(analib gitinfo)

install(TARGETS analib DESTINATION lib)

install(FILES ${PROJECT_BINARY_DIR}/libanalib_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libanalib.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.hh include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
