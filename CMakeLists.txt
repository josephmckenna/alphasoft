##################################################
# Don't play with this unless you know what you  #
# are doing...                                   #
#                                                #
# author: Germano Bonomi germano.bonomi@cern.ch  #
# based on CMake Configuration file for the VMC  #
# E01 example - I. Hrivnacova                    #
##################################################

# To be sure your cmake version in recent enough
cmake_minimum_required(VERSION 2.6.4 FATAL_ERROR)

# Project name
project(a2MC)


#----------------------------------------------------------------------------
# Define unique names of libraries and executables based on project name
#
set(library_name vmc_${PROJECT_NAME})
set(g4library_name geant4_${PROJECT_NAME})
set(program_name ${PROJECT_NAME})
#message(STATUS "library_name = ${library_name}")

#----------------------------------------------------------------------------
# CMake Module Path
#
set(CMAKE_MODULE_PATH
    ${Geant4VMC_DIR}/Modules
    ${CMAKE_MODULE_PATH})

#----------------------------------------------------------------------------
# VMC Configuration file
# (for building MC independent code)
#
include(UseVMC)

#----------------------------------------------------------------------------
# Setup project include directories; compile definitions; link libraries
#
include_directories(
  ${PROJECT_SOURCE_DIR}/include
  ${CMAKE_CURRENT_BINARY_DIR})

#----------------------------------------------------------------------------
# Include the required packages
#
# https://root.cern/manual/integrate_root_into_my_cmake_project/
# https://cliutils.gitlab.io/modern-cmake/chapters/packages/ROOT.html
find_package(ROOT 6.16 CONFIG REQUIRED)
include(${ROOT_USE_FILE})

find_package(Geant4 CONFIG REQUIRED)
include_directories(${Geant4_INCLUDE_DIRS})
find_package(VGM CONFIG REQUIRED)
include_directories(${VGM_INCLUDE_DIRS})
find_package(Geant4VMC CONFIG REQUIRED)
include_directories(${Geant4VMC_INCLUDE_DIRS})
find_package(VMC CONFIG REQUIRED)
include_directories(${VMC_INCLUDE_DIRS})

#----------------------------------------------------------------------------
# Generate Root dictionaries
#
ROOT_GENERATE_DICTIONARY(
  ${CMAKE_SHARED_LIBRARY_PREFIX}${library_name}_dict
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcMessenger.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcStack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcRootManager.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcMCTrack.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcVirtualMC.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcHit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcSilHit.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcSilDIGI.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcSilSD.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcApparatus.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcFieldConstant.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcFieldFromMap.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcPrimary.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcGenerator.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcSettings.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcMuonGen.h
  ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcUtility.h
  MODULE ${library_name}
  LINKDEF ${CMAKE_CURRENT_SOURCE_DIR}/include/a2mcLinkDef.h)

set(GCC_COVERAGE_COMPILE_FLAGS "-g")
add_definitions(${GCC_COVERAGE_COMPILE_FLAGS})

#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cxx)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.h)

#----------------------------------------------------------------------------
# Automatically set the runtime path of your executable to your shared library
#
set(CMAKE_INSTALL_RPATH ${CMAKE_INSTALL_LIBDIR})
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)
#  set the RPATH before defining the targets - so above

#----------------------------------------------------------------------------
# Libraries...fun part
#
add_library(${library_name} ${sources} ${CMAKE_SHARED_LIBRARY_PREFIX}${library_name}_dict.cxx ${headers})
target_link_libraries(${library_name} ${Geant4_LIBRARIES} ${ROOT_LIBRARIES} ${VGM_LIBRARIES} ${VMC_LIBRARIES} ${Geant4VMC_LIBRARIES})

install(TARGETS ${library_name} DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${PROJECT_BINARY_DIR}/lib${library_name}_rdict.pcm DESTINATION ${CMAKE_INSTALL_LIBDIR})
install(FILES ${PROJECT_BINARY_DIR}/lib${library_name}.rootmap DESTINATION ${CMAKE_INSTALL_LIBDIR})

# Create the executables and link it
add_executable(g4vmc_${program_name} ${program_name}.cxx)
target_link_libraries(g4vmc_${program_name} ${library_name})

add_custom_target(${PROJECT_NAME} DEPENDS
                  g4vmc_${program_name})
install(TARGETS g4vmc_${program_name} DESTINATION ${CMAKE_INSTALL_PREFIX})

configure_file( ${PROJECT_SOURCE_DIR}/input/a2MC.ini
                ${CMAKE_INSTALL_PREFIX}
		COPYONLY )
