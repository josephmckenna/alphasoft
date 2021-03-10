# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(aged)


find_package(X11 REQUIRED)
if(X11_FOUND)
  include_directories(${X11_INCLUDE_DIR})
  message(STATUS "X11 headers: ${X11_INCLUDE_DIR}")
  message(STATUS "X11 libs: ${X11_X11_LIB} ${X11_Xrender_LIB} ${X11_Xmu_LIB} ${X11_Xft_LIB} ${X11_Xt_LIB}")
else()
  message(FATAL "X11 not found")
endif(X11_FOUND)

find_package(Motif REQUIRED)
if(MOTIF_FOUND)
  include_directories(${MOTIF_INCLUDE_DIR})
  message(STATUS "Motif headers: ${MOTIF_INCLUDE_DIR}")
  message(STATUS "Motif libs: ${MOTIF_LIBRARIES}")
else()
  message(FATAL "Motif or lesstif not found")
endif(MOTIF_FOUND)

find_package(Freetype REQUIRED)
if(FREETYPE_FOUND)
  include_directories(${FREETYPE_INCLUDE_DIR_freetype2})
  message(STATUS "Freetype ${FREETYPE_VERSION_STRING}")
  message(STATUS "Freetype headers: ${FREETYPE_INCLUDE_DIR_freetype2}")
  message(STATUS "Freetype libs: ${FREETYPE_LIBRARIES}")
else()
  message(FATAL "Freetype not found")
endif(FREETYPE_FOUND)

#find_package(Fontconfig REQUIRED)
#if(FONTCONFIG_FOUND)
#  include_directories(${Fontconfig_INCLUDE_DIRS}/fontconfig)
#  message(STATUS "Fontconfig ${Fontconfig_VERSION}")
#  message(STATUS "Fontconfig headers: ${Fontconfig_INCLUDE_DIRS}/fontconfig")
#  message(STATUS "Fontconfig libs: ${Fontconfig_LIBRARIES}")
#else()
#  message(FATAL "Fontconfig not found")
#endif(FONTCONFIG_FOUND)
#
#find_package(PNG REQUIRED)
#if(PNG_FOUND)
#  include_directories(${PNG_INCLUDE_DIRS})
#  add_definitions(${PNG_DEFINITIONS})
#  message(STATUS "PNG ${PNG_VERSION_STRING}")
#  message(STATUS "PNG headers: ${PNG_INCLUDE_DIRS}")
#  message(STATUS "PNG libs ${PNG_LIBRARIES}")
#else()
#  message(FATAL "PNG not found")
#endif(PNG_FOUND)
#
#find_package(JPEG REQUIRED)
#if(JPEG_FOUND)
#  include_directories(${JPEG_INCLUDE_DIRS})
#  message(STATUS "JPEG ${JPEG_VERSION}")
#  message(STATUS "JPEG headers: ${JPEG_INCLUDE_DIRS}")
#  message(STATUS "JPEG libs ${JPEG_LIBRARIES}")
#else()
#  message(FATAL "JPEG not found")
#endif(JPEG_FOUND)


add_compile_options(-Wall -Wextra -pedantic -Werror)

if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 
  add_definitions(-DANTI_ALIAS)
else()
  add_compile_definitions(ANTI_ALIAS)
endif()

add_link_options(-lm -ldl -rdynamic)

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

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx)
add_library(aged SHARED ${LIBRARY_SOURCES})

if(ROOTANA_SUBMOD)
  add_dependencies(aged rootana)
endif()

target_include_directories(aged PUBLIC include) 
#target_link_libraries(aged ${MOTIF_LIBRARIES} ${X11_X11_LIB} ${X11_Xrender_LIB} ${X11_Xmu_LIB} ${X11_Xft_LIB} ${X11_Xt_LIB} ${FREETYPE_LIBRARIES} ${Fontconfig_LIBRARIES} ${PNG_LIBRARIES} ${JPEG_LIBRARIES})
target_link_libraries(aged ${MOTIF_LIBRARIES} ${X11_X11_LIB} ${X11_Xrender_LIB} ${X11_Xmu_LIB} ${X11_Xft_LIB} ${X11_Xt_LIB} ${FREETYPE_LIBRARIES})### ${ROOT_LIBRARIES}) # ROOT libraries actually not necessary

install(TARGETS aged DESTINATION lib)
file(GLOB HEADER_FILES include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
