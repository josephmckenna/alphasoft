
file(GLOB MainMacros main*.cxx)
foreach( MainMacro ${MainMacros} )
  string( REPLACE ".cxx" ".exe" bin ${MainMacro} )
  get_filename_component(bin ${bin} NAME)
  add_executable( macro_${bin}  ${MainMacro} )
  target_include_directories(macro_${bin} PUBLIC ${CMAKE_SOURCE_DIR}/rootUtils/include)

  target_include_directories(macro_${bin} PUBLIC .)
  target_include_directories(macro_${bin} PUBLIC ${CMAKE_SOURCE_DIR}/a2lib/include)
  target_include_directories(macro_${bin} PUBLIC ${CMAKE_SOURCE_DIR}/recolib/include)
  target_include_directories(macro_${bin} PUBLIC ${CMAKE_SOURCE_DIR}/analib/include)
  target_include_directories(macro_${bin} PUBLIC ${CMAKE_SOURCE_DIR}/ana/include)
  target_include_directories(macro_${bin} PUBLIC ${CMAKE_SOURCE_DIR}/libGEM/include)

  target_link_libraries( macro_${bin} ${A2_LINK_LIBS} rootUtils
     ${SQLite3_LIBRARIES}
     ${ROOT_LIBRARIES}
     ${MIDAS_LIBRARIES} util) 

  install(TARGETS macro_${bin} DESTINATION "${CMAKE_INSTALL_PREFIX}" )

endforeach( MainMacro ${MainMacros} )
