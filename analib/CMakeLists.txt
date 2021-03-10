# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(analib)

set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ana/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)
if (BUILT_IN_ROOTANA)
   include_directories(${CMAKE_SOURCE_DIR}/rootana/manalyzer)
   include_directories(${CMAKE_SOURCE_DIR}/rootana/midasio)
   include_directories(${CMAKE_SOURCE_DIR}/rootana/mvodb)
   include_directories(${CMAKE_BINARY_DIR}/rootana/include)
else ()
   include_directories($ENV{ROOTANASYS}/include)
endif (BUILT_IN_ROOTANA)

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.h include/*.hh)
ROOT_GENERATE_DICTIONARY(G__analib ${LINK_HEADER_FILES} LINKDEF LibLinkDef.hh)

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.c src/*.cxx)
add_library(analib SHARED ${LIBRARY_SOURCES} G__analib.cxx)
add_dependencies(analib gitinfo)
add_dependencies(analib rootana)

install(TARGETS analib DESTINATION lib)

install(FILES ${PROJECT_BINARY_DIR}/libanalib_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libanalib.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.hh include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
