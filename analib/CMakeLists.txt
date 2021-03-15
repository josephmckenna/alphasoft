# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(analib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include)
include_directories(${CMAKE_BINARY_DIR})
include_directories(${CMAKE_SOURCE_DIR}/ana/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)
include_directories(${ROOTANA_INCLUDE_DIRS})

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.h include/*.hh)
ROOT_GENERATE_DICTIONARY(G__analib ${LINK_HEADER_FILES} LINKDEF LibLinkDef.hh)

# Create a shared library with geneated dictionary
set(LIBRARY_SOURCES 
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Sequencer2.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TAnalysisReport.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TChronoChannelName.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TChrono_Event.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TSeq_Dump.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TSeq_Event.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TSequencerDriver.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TSequencerState.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TSpill.cxx
    ${CMAKE_CURRENT_SOURCE_DIR}/src/TStoreLabVIEWEvent.cxx)
if(BUILD_AG)
  list(APPEND LIBRARY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/TAGSpill.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/TStoreEvent.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/TStoreHelix.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/TStoreLine.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/TBarEvent.cxx)
endif()
if(BUILD_A2)
  list(APPEND LIBRARY_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/src/TA2Spill.cxx ${CMAKE_CURRENT_SOURCE_DIR}/src/sqlite3.c)
endif()

add_library(analib SHARED ${LIBRARY_SOURCES} G__analib.cxx)
add_dependencies(analib rootana)

install(TARGETS analib DESTINATION lib)

install(FILES ${PROJECT_BINARY_DIR}/libanalib_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libanalib.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.hh include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
