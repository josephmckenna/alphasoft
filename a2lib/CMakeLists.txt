# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alpha2lib)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Include directories
include_directories(include ${ROOT_INCLUDE_DIRS})
if (BUILT_IN_ROOTANA)
   include_directories(${CMAKE_SOURCE_DIR}/rootana/include)
else ()
   include_directories($ENV{ROOTANASYS}/include)
endif (BUILT_IN_ROOTANA)

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.h)
ROOT_GENERATE_DICTIONARY(G__alpha2 ${LINK_HEADER_FILES} LINKDEF LibLinkDef.hh )

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx)
file(GLOB C_SOURCES src/*.c)

add_library(alpha2 SHARED ${LIBRARY_SOURCES} ${C_SOURCES} G__alpha2.cxx )
target_include_directories(alpha2 PUBLIC include)

find_library(LIBSQLITE3 sqlite3)
#target_link_libraries(alpha2 ${ROOT_LIBRARIES} ${LIBSQLITE3} )
target_link_libraries(alpha2 ${LIBSQLITE3} )

if(ROOTANA_SUBMOD)
  add_dependencies(alpha2 rootana)
endif(ROOTANA_SUBMOD)

install(TARGETS alpha2 DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libalpha2_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libalpha2.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.db DESTINATION db)

