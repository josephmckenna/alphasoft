
find_package(Garfield)
if (GARFIELD_FOUND)
message(STATUS "Garfield found...")
option(BUILT_IN_GARFIELD "Use built in Garfieldpp" OFF)
else()
message(STATUS "GARFILED NOT FOUND")
option(BUILT_IN_GARFIELD "Use built in Garfieldpp" ON)
endif()



add_subdirectory(common)
add_subdirectory(CRY)
add_subdirectory(garfieldpp)
add_subdirectory(geant4)
add_subdirectory(agg4)
