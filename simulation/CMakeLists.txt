
#----------------------------------------------------------------------------
# Garfield++
#
if(EXISTS $ENV{GARFIELD_HOME}/lib/cmake/Garfield/GarfieldConfig.cmake)
  list(APPEND CMAKE_PREFIX_PATH $ENV{GARFIELD_HOME}/lib/cmake/Garfield)
endif()

find_package(Garfield REQUIRED)
if (Garfield_FOUND)
  message(STATUS "Garfield found...")
  message(STATUS "Garfield headers: ${GARFIELD_INCLUDE_DIR}")
else()
  message(FATAL_ERROR "GARFILED NOT FOUND")
endif()


#----------------------------------------------------------------------------

add_subdirectory(common)
add_subdirectory(CRY)
get_submodule(CADMesh)
add_subdirectory(CADMesh)
add_subdirectory(garfieldpp)
add_subdirectory(geant4)
add_subdirectory(agg4)
