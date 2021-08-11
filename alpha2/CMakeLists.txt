# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alpha2)
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

macro(replace_list_item LIST OLD_VALUE NEW_VALUE)
    list(FIND ${LIST} ${OLD_VALUE} OLD_VALUE_INDEX)
    if(OLD_VALUE_INDEX GREATER_EQUAL 0)
        list(REMOVE_AT ${LIST} ${OLD_VALUE_INDEX})
        list(INSERT ${LIST} ${OLD_VALUE_INDEX} ${NEW_VALUE})
    endif()
endmacro()


#Extra path for GitInfo.h file
include_directories(${CMAKE_BINARY_DIR})

if (BUILT_IN_ROOTANA)
  message(STATUS "Using ROOTANA submodule for A2")
else()
  find_package(ROOTANA)
endif(BUILT_IN_ROOTANA)
message(STATUS "ROOTANA headers for A2: ${ROOTANA_INCLUDE_DIRS}")

#Find all *.modules files
file(GLOB BINS *.modules)

if(BUILD_A2)
  set(A2_LINK_LIBS alpha2 analib GEM TMVA TMVAGui)
endif()

#message(Binaries: ${BINS})
foreach( LIST_FILE ${BINS} )
    string( REPLACE ".modules" ".exe" bin ${LIST_FILE} )
    get_filename_component(bin ${bin} NAME)
    message("alpha2 binary: " ${bin})
    
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

    if (BUILT_IN_ROOTANA)
       add_executable( ${bin} ../rootana/manalyzer/manalyzer_main  ${MODULES} )
       add_dependencies(${bin} rootana)
    else ()
       add_executable( ${bin} $ENV{ROOTANASYS}/manalyzer/manalyzer_main  ${MODULES} )
    endif (BUILT_IN_ROOTANA)
    
    #add_dependencies(${bin} alpha2 agtpc analib GEM)
    add_dependencies(${bin} alpha2 analib GEM)
    #add_dependencies(${bin} gitinfo)

    # Make sure YourLib is linked to each app
    target_include_directories(${bin} PUBLIC include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/a2lib/include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/recolib/include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/analib/include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/ana/include)
    target_include_directories(${bin} PUBLIC ${CMAKE_SOURCE_DIR}/libGEM/include)
    target_include_directories(${bin} PUBLIC ${ROOT_INCLUDE_DIRS})
    target_include_directories(${bin} PUBLIC ${ROOTANA_INCLUDE_DIRS})
    target_include_directories(${bin} PUBLIC ${MIDAS_INCLUDE_DIRS})
   
    target_link_libraries( ${bin} ${A2_LINK_LIBS}
                           ${SQLite3_LIBRARIES}
                           rootana ${ROOT_LIBRARIES}
			   ${MIDAS_LIBRARIES} util) 
    
    install(TARGETS ${bin}  DESTINATION "${CMAKE_INSTALL_PREFIX}" )
endforeach( LIST_FILE ${BINS} )

add_subdirectory(macros)