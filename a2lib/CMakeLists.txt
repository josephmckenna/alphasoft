# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(alpha2lib)
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(ROOT REQUIRED COMPONENTS Minuit2 Geom)

# Include directories
include_directories(include 
                    ${ROOT_INCLUDE_DIRS})

add_custom_target(
    sis_database ALL
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/main.db
    )
add_custom_command(
     OUTPUT
    #${CMAKE_CURRENT_BINARY_DIR}/__header.h  # fake! ensure we run!
    ${CMAKE_CURRENT_SOURCE_DIR}/main.db    # real header, we write.
    #this command must generate: ${CMAKE_CURRENT_BINARY_DIR}/header.h
    COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/build_db.sh 
    )

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.h)
ROOT_GENERATE_DICTIONARY(G__alpha2 ${LINK_HEADER_FILES} LINKDEF LibLinkDef.hh )

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx)
file(GLOB C_SOURCES src/*.c)

add_library(alpha2 SHARED ${LIBRARY_SOURCES} ${C_SOURCES} G__alpha2.cxx )

target_include_directories(alpha2 PUBLIC include)
target_include_directories(alpha2 PUBLIC ${SQLite3_INCLUDE_DIRS})
target_link_libraries(alpha2 PUBLIC manalyzer Graf Matrix Tree Graf3d Physics Minuit2 Geom ${SQLite3_LIBRARIES})
add_dependencies(alpha2 sis_database)

install(TARGETS alpha2 DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libalpha2_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libalpha2.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.h)
install(FILES ${HEADER_FILES} DESTINATION include)
install(FILES ${CMAKE_CURRENT_SOURCE_DIR}/main.db DESTINATION db)
