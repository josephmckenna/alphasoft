# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alphasoft)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

macro(replace_list_item LIST OLD_VALUE NEW_VALUE)
    list(FIND ${LIST} ${OLD_VALUE} OLD_VALUE_INDEX)
    if(OLD_VALUE_INDEX GREATER_EQUAL 0)
        list(REMOVE_AT ${LIST} ${OLD_VALUE_INDEX})
        list(INSERT ${LIST} ${OLD_VALUE_INDEX} ${NEW_VALUE})
    endif()
endmacro()


set(N_PAD_DECONV_THREADS "2" CACHE STRING "N Threads for Pad Deconvolution")
set_property(CACHE N_PAD_DECONV_THREADS PROPERTY STRINGS "1" "2" "3" "4" "5" "8")

set(N_KDTREE_MATCH_THREADS "2" CACHE STRING "N Threads for KDTree spacepoint finding")
set_property(CACHE N_KDTREE_MATCH_THREADS PROPERTY STRINGS "1" "2")

set(N_TRACK_FIT_THREADS "2" CACHE STRING "N Threads for Track Fitting (Minuit2 only)")
set_property(CACHE N_TRACK_FIT_THREADS PROPERTY STRINGS "1" "2" "3" "4" "8")

set(N_VERTEX_FIT_THREADS "1" CACHE STRING "N Threads for Vertex fitting (each phase)")
set_property(CACHE N_VERTEX_FIT_THREADS PROPERTY STRINGS "1" "2" "3")

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

#Extra path for GitInfo.h file
include_directories(${CMAKE_BINARY_DIR})

#Find all *.modules files
file(GLOB BINS *.modules)

# ALPHA-g libraries
if(BUILD_AG)
  set(AG_LINK_LIBS agana aged agtpc aglib analib ${SQLite3_LIBRARIES})
endif()


#set(MINILIBS "")
foreach( LIST_FILE ${BINS} )
   
    #message(List file: ${LIST_FILE})
    execute_process( COMMAND grep -v "\#" ${LIST_FILE} 
                     OUTPUT_VARIABLE MODULES )
    #Clean up any strange formatting choices
    string( REPLACE "\n" " " MODULES ${MODULES} )
    string( REPLACE "\t" " " MODULES ${MODULES} )
    #.o files come from .cxx files... cmake wants this
    string( REPLACE ".o" "" MODULES ${MODULES} )

    #Turn string into list
    separate_arguments(MODULES)
    #Prepend path to source files
    #list(TRANSFORM MODULES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/src/)
    foreach ( MODULE ${MODULES})
       list(APPEND MINI_LIBS ${MODULE}) 
    endforeach()
endforeach( LIST_FILE ${BINS} )

list(REMOVE_DUPLICATES MINI_LIBS)
list(REMOVE_ITEM MINI_LIBS "${CMAKE_CURRENT_SOURCE_DIR}/src/")

foreach ( L ${MINI_LIBS})
    # I am not totally happy that I am hard coding module names and matching compiler settings
    # but it does keep 1:1 connection between these which would be good for build speeds when not much has changed
    add_library(${L} OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/${L}.cxx)
    target_include_directories(${L} PUBLIC include)
    target_link_libraries(${L} PUBLIC ${AG_LINK_LIBS})
    if (${L} STREQUAL "deconv_pad_module")
        target_compile_definitions(${L} PRIVATE N_PAD_DECONV_THREADS=${N_PAD_DECONV_THREADS})
    elseif(${L} STREQUAL "kdtreematch")
        target_compile_definitions(${L} PRIVATE N_KDTREE_MATCH_THREADS=${N_KDTREE_MATCH_THREADS})
    elseif(${L} STREQUAL "reco_trackfitter")
        target_compile_definitions(${L} PRIVATE N_TRACK_FIT_THREADS=${N_TRACK_FIT_THREADS})
    elseif(${L} STREQUAL "reco_vertexfitter")
        target_compile_definitions(${L} PRIVATE N_VERTEX_FIT_THREADS=${N_VERTEX_FIT_THREADS})
    endif()
    endforeach()


#message(Binaries: ${BINS})
foreach( LIST_FILE ${BINS} )
    string( REPLACE ".modules" ".exe" bin ${LIST_FILE} )
    get_filename_component(bin ${bin} NAME)
    #message("agana binary: " ${bin})
    
    #message(List file: ${LIST_FILE})
    execute_process( COMMAND grep -v "\#" ${LIST_FILE} 
                     OUTPUT_VARIABLE MODULES )
    #Clean up any strange formatting choices
    string( REPLACE "\n" " " MODULES ${MODULES} )
    string( REPLACE "\t" " " MODULES ${MODULES} )
    #.o files come from .cxx files... cmake wants this
    string( REPLACE ".o" "" MODULES ${MODULES} )

    #Turn string into list
    separate_arguments(MODULES)

    file(WRITE ${CMAKE_BINARY_DIR}/${bin}.cpp "")
    add_executable( ${bin} ${CMAKE_BINARY_DIR}/${bin}.cpp)
    

    target_include_directories(${bin} PUBLIC include)

    if( MIDAS_FOUND )
      link_directories(${bin} PUBLIC ${MIDAS_LIBRARY_DIRS})
      target_include_directories(${bin} PUBLIC ${MIDAS_INCLUDE_DIRS})
    endif()

    target_link_libraries(${bin} PUBLIC ${MODULES})

    target_link_libraries( ${bin} PUBLIC # ${AG_LINK_LIBS}
                           ${ZLIB_LIBRARIES}
                           manalyzer_main
                           ${MIDAS_LIBRARIES})# ${MODULES}) 
    
    install(TARGETS ${bin}  DESTINATION "${CMAKE_INSTALL_PREFIX}" )
endforeach( LIST_FILE ${BINS} )
