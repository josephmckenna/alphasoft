# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(aglib)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
include_directories(${CMAKE_SOURCE_DIR}/a2lib/include)

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.h include/*.hh)
ROOT_GENERATE_DICTIONARY(G__aglib ${LINK_HEADER_FILES} LINKDEF LibLinkDef.hh)

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx)
    
add_library(aglib SHARED ${LIBRARY_SOURCES} G__aglib.cxx)
target_include_directories(aglib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)

target_link_libraries(aglib PUBLIC agtpc manalyzer Core RIO Hist)

install(TARGETS aglib DESTINATION lib)

install(FILES ${PROJECT_BINARY_DIR}/libaglib_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libaglib.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.hh include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
