# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alphagana)
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


set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")

#Find all *.modules files
file(GLOB BINS *.modules)

# ALPHA-g libraries
if(BUILD_AG)
  set(AG_LINK_LIBS agana aged agtpc aglib analib)
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
   #message("L:" ${L})
   add_library(${L} OBJECT ${CMAKE_CURRENT_SOURCE_DIR}/src/${L}.cxx)
   target_include_directories(${L} PUBLIC include)
   target_link_libraries(${L} PUBLIC ${AG_LINK_LIBS})
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
