# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alphasoft)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

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

#Extra path for GitInfo.h file
include_directories(${CMAKE_BINARY_DIR})

set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

#Find all *.modules files
file(GLOB BINS *.modules)

# ALPHA-g libraries
set(AG_LINK_LIBS agana aged agtpc analib)

# alphasoft 
message(Binaries: ${BINS})
foreach( LIST_FILE ${BINS} )
    string( REPLACE ".modules" ".exe" bin ${LIST_FILE} )
    get_filename_component(bin ${bin} NAME)
    message("agana binary: " ${bin})
    
    #message(List file: ${LIST_FILE})
    execute_process( COMMAND grep -v "\#" ${LIST_FILE} 
                     OUTPUT_VARIABLE MODULES )
    #Clean up any strange formatting choices
    string( REPLACE "\n" " " MODULES ${MODULES} )
    string( REPLACE "\t" " " MODULES ${MODULES} )
    #.o files come from .cxx files... cmake wants this
    string( REPLACE ".o" ".cxx" MODULES ${MODULES} )

    #Turn string into list
    separate_arguments(MODULES)
    #Prepend path to source files
    list(TRANSFORM MODULES PREPEND ${CMAKE_CURRENT_SOURCE_DIR}/src/)

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/unpack_module.cxx" 
        "../agana/unpack_module.cxx"
    )

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/pwb_module.cxx" 
        "../agana/pwb_module.cxx"
    )

    replace_list_item(
        MODULES 
        "${CMAKE_CURRENT_SOURCE_DIR}/src/adc_module.cxx" 
        "../agana/adc_module.cxx"
    )

    #########################################################
    add_executable(${bin} ${MODULES})
    #########################################################
    
    add_dependencies(${bin} agana analib agtpc aged)

    target_compile_definitions(${bin} PUBLIC ${ROOT_CCX_FLAGS} HAVE_ROOT HAVE_ROOT_HTTP HAVE_THTTP_SERVER)
    
    target_include_directories(${bin} PUBLIC include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/recolib/include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/analib/include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/aged/include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/agana)

    target_compile_definitions(${bin} PRIVATE N_PAD_DECONV_THREADS=${N_PAD_DECONV_THREADS})
    target_compile_definitions(${bin} PRIVATE N_KDTREE_MATCH_THREADS=${N_KDTREE_MATCH_THREADS})
    target_compile_definitions(${bin} PRIVATE N_TRACK_FIT_THREADS=${N_TRACK_FIT_THREADS})
    target_compile_definitions(${bin} PRIVATE N_VERTEX_FIT_THREADS=${N_VERTEX_FIT_THREADS})

    target_link_libraries( ${bin} ${AG_LINK_LIBS}
                           ${SQLite3_LIBRARIES}
                           ${ROOT_LIBRARIES}
                           ${ZLIB_LIBRARIES}
			   manalyzer manalyzer_main
			 ) 
    
    install(TARGETS ${bin}  DESTINATION "${CMAKE_INSTALL_PREFIX}" )
endforeach( LIST_FILE ${BINS} )
