cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(g4out)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

file(GLOB headers ${PROJECT_SOURCE_DIR}/include/T*.hh)
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
include_directories(${PROJECT_SOURCE_DIR}/include)

# locate ROOT
find_package(ROOT REQUIRED)
include_directories(${ROOT_INCLUDE_DIR})
# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

ROOT_GENERATE_DICTIONARY(G__G4out ${headers} LINKDEF include/G4outLinkDef.hh)
add_library(G4out SHARED ${sources} G__G4out)

target_link_libraries(G4out PUBLIC Core agtpc)

install(TARGETS G4out DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libG4out_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libG4out.rootmap DESTINATION lib)
install(FILES ${headers} DESTINATION include)

