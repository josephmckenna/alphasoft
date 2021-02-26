# For a while I considered storing stl files in a tag.gz... this is how I extracted it...
# but considering git has its own compression, its likely better to just keep the ASCII files in the git directly

   #Really nice cmake 3.18 tool, but cmake 3.18 isn't widely adopted yet:
   #file(ARCHIVE_EXTRACT INPUT CAD_Files.tar.gz DESTINATION ${CMAKE_INSTALL_PREFIX}/simulation)

   #if(EXISTS "${CMAKE_CURRENT_BINARY_DIR}/0_ASCII.stl")
   #   message (STATUS "AG CAD files already unpacked")
   #else()
   #   message (STATUS "Unpacking CAD files")
   #   execute_process(COMMAND ${CMAKE_COMMAND} -E tar xvz "${CMAKE_CURRENT_SOURCE_DIR}/CAD_Files.tar.gz" WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR} OUTPUT_VARIABLE STL_FILES OUTPUT_STRIP_TRAILING_WHITESPACE )
   #endif()
  #message(${AG_CAD_FILE_PATH})


set(AG_CAD_FILE_PATH "${CMAKE_INSTALL_PREFIX}/simulation/AG_CAD_Files/" CACHE INTERNAL "")

file(GLOB STL_FILESS ${CMAKE_CURRENT_SOURCE_DIR}/CAD_Files/*ASCII.stl ${CMAKE_CURRENT_SOURCE_DIR}/CAD_Files/cad_part_list.txt)
install(FILES ${STL_FILESS} DESTINATION ${AG_CAD_FILE_PATH})

