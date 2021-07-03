# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(aged)

find_package(ROOT REQUIRED)

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

if(${CMAKE_VERSION} VERSION_LESS "3.12.0") 
  add_definitions(-DANTI_ALIAS)
else()
  add_compile_definitions(ANTI_ALIAS)
endif()

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/agana)
include_directories(${CMAKE_SOURCE_DIR}/analib/include)
include_directories(${CMAKE_SOURCE_DIR}/ana/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)
if(BUILD_MANALYZER)
  include_directories(${CMAKE_SOURCE_DIR}/manalyzer)
  include_directories(${CMAKE_SOURCE_DIR}/midasio)
  include_directories(${CMAKE_SOURCE_DIR}/mvodb)
elseif(MIDAS_FOUND)
  include_directories(${MIDAS_INCLUDE_DIRS})
  include_directories($ENV{MIDASSYS}/manalyzer)
else()
  message(FATAL_ERROR "aged: Include directories failed!")
endif()


# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx)
add_library(aged SHARED ${LIBRARY_SOURCES})

target_include_directories(aged PUBLIC include ${X11_INCLUDE_DIR} ) 
target_link_libraries(aged PUBLIC ${ROOT_LIBRARIES} ${MOTIF_LIBRARIES} ${X11_X11_LIB} ${X11_Xrender_LIB} ${X11_Xmu_LIB} ${X11_Xft_LIB} ${X11_Xt_LIB} ${FREETYPE_LIBRARIES})

install(TARGETS aged DESTINATION lib)
file(GLOB HEADER_FILES include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
