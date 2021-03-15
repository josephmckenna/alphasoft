# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(rootutils)

find_package (Python COMPONENTS Interpreter Development)
if (${Python_FOUND})
   message("Python found! Enabling python interface")
   add_definitions(-DHAVE_PYTHON)
   include_directories(${Python_INCLUDE_DIRS})
   link_libraries(${Python_})
endif()

include_directories(
   include 
   ${ROOT_INCLUDE_DIRS} 
   ${ROOTANA_INCLUDE_DIRS}
   ${CMAKE_SOURCE_DIR}/libGEM/include
)
file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.h)
ROOT_GENERATE_DICTIONARY(G__rootUtils ${LINK_HEADER_FILES} LINKDEF LibLinkDef.hh)

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.c src/*.cxx)
add_library(rootUtils SHARED ${LIBRARY_SOURCES} G__rootUtils.cxx )
target_include_directories( rootUtils
   PUBLIC
      include 
      ${CMAKE_SOURCE_DIR}/ana/include
      ${CMAKE_SOURCE_DIR}/libGEM/include
      ${CMAKE_SOURCE_DIR}/recolib/include
      ${CMAKE_SOURCE_DIR}/analib/include
      ${CMAKE_SOURCE_DIR}/a2lib/include
   )

if (${Python_FOUND})
   target_link_libraries(rootUtils ${Python_LIBRARIES})
endif()

install(TARGETS rootUtils 
  LIBRARY
      DESTINATION lib 
   PUBLIC_HEADER
      DESTINATION include
   )
install(FILES ${PROJECT_BINARY_DIR}/librootUtils_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/librootUtils.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
