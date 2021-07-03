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


#Extra path for GitInfo.h file
include_directories(${CMAKE_BINARY_DIR})

#Find all *.modules files
file(GLOB BINS *.modules)

# ALPHA-g libraries
set(AG_LINK_LIBS agana aged agtpc analib)

# manalyzer setup
if(BUILD_MANALYZER)
  set(manalyzer_objs
    ${CMAKE_SOURCE_DIR}/midasio/midasio.cxx
    ${CMAKE_SOURCE_DIR}/midasio/lz4frame.c
    ${CMAKE_SOURCE_DIR}/midasio/xxhash.c
    ${CMAKE_SOURCE_DIR}/mvodb/mvodb.cxx
    ${CMAKE_SOURCE_DIR}/mvodb/nullodb.cxx
    ${CMAKE_SOURCE_DIR}/mvodb/mjsonodb.cxx
    ${CMAKE_SOURCE_DIR}/mvodb/mxmlodb.cxx
    ${CMAKE_SOURCE_DIR}/mjson/mjson.cxx
    ${CMAKE_SOURCE_DIR}/mxml/mxml.cxx
    )


  add_library(manalyzer      STATIC ${CMAKE_SOURCE_DIR}/manalyzer/manalyzer ${manalyzer_objs})
  add_library(manalyzer_main STATIC ${CMAKE_SOURCE_DIR}/manalyzer/manalyzer_main)
  
  target_include_directories(manalyzer      PUBLIC ${CMAKE_SOURCE_DIR}/manalyzer)
  target_include_directories(manalyzer_main PUBLIC ${CMAKE_SOURCE_DIR}/manalyzer)
  
  target_compile_features(manalyzer      PUBLIC cxx_std_11)
  target_compile_features(manalyzer_main PUBLIC cxx_std_11)

  if(MIDAS_FOUND)
    target_compile_definitions(manalyzer      PUBLIC -DHAVE_MIDAS -DHAVE_TMFE)
    target_compile_definitions(manalyzer_main PUBLIC -DHAVE_MIDAS -DHAVE_TMFE)
    target_link_libraries(manalyzer      PUBLIC midas)
    target_link_libraries(manalyzer_main PUBLIC midas)
  else()
    target_include_directories(manalyzer      PUBLIC ${CMAKE_SOURCE_DIR}/midasio ${CMAKE_SOURCE_DIR}/mvodb ${CMAKE_SOURCE_DIR}/mjson ${CMAKE_SOURCE_DIR}/mxml)
    target_include_directories(manalyzer_main PUBLIC ${CMAKE_SOURCE_DIR}/midasio ${CMAKE_SOURCE_DIR}/mvodb)
  endif()

  target_include_directories(manalyzer      PUBLIC ${ROOT_INCLUDE_DIRS})
  target_include_directories(manalyzer_main PUBLIC ${ROOT_INCLUDE_DIRS})
  target_compile_definitions(manalyzer      PUBLIC ${ROOT_CCX_FLAGS} -DHAVE_ROOT)
  target_compile_definitions(manalyzer_main PUBLIC ${ROOT_CCX_FLAGS} -DHAVE_ROOT)
  target_compile_definitions(manalyzer      PUBLIC -DHAVE_ROOT_HTTP -DHAVE_THTTP_SERVER)
  target_compile_definitions(manalyzer_main PUBLIC -DHAVE_ROOT_HTTP -DHAVE_THTTP_SERVER)
endif()

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
    target_include_directories(${bin} PUBLIC ${ROOT_INCLUDE_DIRS})
    if( MIDAS_FOUND )
      target_link_directories(${bin} PUBLIC ${MIDAS_LIBRARY_DIRS})
      target_include_directories(${bin} PUBLIC ${MIDAS_INCLUDE_DIRS})
    endif()

    if(BUILD_MANALYZER)
      target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/manalyzer ${CMAKE_SOURCE_DIR}/midasio ${CMAKE_SOURCE_DIR}/mvodb ${CMAKE_SOURCE_DIR}/mjson ${CMAKE_SOURCE_DIR}/mxml)   
    endif()

    target_link_libraries( ${bin} ${AG_LINK_LIBS}
                           ${SQLite3_LIBRARIES}
                           ${ROOT_LIBRARIES}
			   manalyzer manalyzer_main
			   ${MIDAS_LIBRARIES}) 
    
    install(TARGETS ${bin}  DESTINATION "${CMAKE_INSTALL_PREFIX}" )
endforeach( LIST_FILE ${BINS} )
