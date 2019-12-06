# CMakeLists.txt for basic analysis package. It creates a library with dictionary and a main program
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(rootSkeleton)

# Version numbering
set (basicApp_VERSION_MAJOR 1)
set (basicApp_VERSION_MAJOR 0)
set (CMAKE_CXX_FLAGS "-O2")
set (CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Enforce out-of-source builds 
string(COMPARE EQUAL "${CMAKE_SOURCE_DIR}" "${CMAKE_BINARY_DIR}" _insource)
if(_insource)
   message(FATAL_ERROR "Error: In-source build detected! Please create a build directory!")
endif(_insource) 

# Checks $CLICOLOR_FORCE variable to force compiler to emit coloured output
# The parameter is based on compiler ID as it's not standardised
if ($ENV{CLICOLOR_FORCE})
    if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
       add_compile_options (-fdiagnostics-color=always)
    elseif ("${CMAKE_CXX_COMPILER_ID}" MATCHES "^(Clang|AppleClang)$")
       add_compile_options (-fcolor-diagnostics)
    endif ()
endif ()

# Directories
add_subdirectory(src)

# You need to tell CMake where to find the ROOT installation. This can be done in a number of ways:
#   - ROOT built with classic configure/make use the provided $ROOTSYS/etc/cmake/FindROOT.cmake
#   - ROOT built with CMake. Add in CMAKE_PREFIX_PATH the installation prefix for ROOT
if(EXISTS $ENV{ROOTSYS}/ROOTConfig.cmake)
    if(NOT DEFINED ENV{CMAKE_PREFIX_PATH})
        list(APPEND CMAKE_PREFIX_PATH $ENV{ROOTSYS})
    endif()
endif()
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "$ENV{ROOTSYS}/etc/cmake")

# Locate the ROOT package and defines a number of variables (e.g. ROOT_INCLUDE_DIRS)
find_package(ROOT REQUIRED COMPONENTS Spectrum)
link_libraries(${ROOT_LIBRARIES})

##Horrible horrible hack to get the libs for rootana (from rootana/include/rootana_cflags.txt )
list(APPEND ROOTANALIBS z Gui Core Imt RIO Net Hist Graf Graf3d Gpad ROOTVecOps Tree TreePlayer Rint Postscript Matrix Physics MathCore Thread MultiProc ROOTDataFrame m dl Thread XMLParser XMLIO RHTTP XMLParser XMLIO RHTTP)
link_libraries(${ROOTANALIBS})


find_package(X11 REQUIRED)
link_libraries(${X11_LIBRARIES})
include_directories(${X11_INCLUDE_DIR})

#link_libraries(agana AGTPC aged)
link_directories($ENV{ROOTANASYS}/lib)

# analib/ AGTPC aged)
#link_directories(analib/ recolib/ aged/)
# Define useful ROOT functions and macros (e.g. ROOT_GENERATE_DICTIONARY)
include(${ROOT_USE_FILE})

# Include directories
include_directories(include)
include_directories($ENV{AGRELEASE}/recolib/include)
include_directories($ENV{AGRELEASE}/analib/include)
include_directories($ENV{AGRELEASE}/aged/include)
include_directories(${ROOT_INCLUDE_DIRS})

#Analysis Report needs A2Flow in alpha2...
include_directories($ENV{AGRELEASE}/alpha2/include)
#...which then needs UnpackVF48 in a2lib
include_directories($ENV{AGRELEASE}/a2lib/include)
include_directories(include $ENV{ROOTANASYS}/include)

# Create a shared library with geneated dictionary
#file(GLOB LIBRARY_SOURCES src/*.cxx RootUtils/*.cxx)
#add_library(agana SHARED ${LIBRARY_SOURCES})
#target_link_libraries(agana ${ROOT_LIBRARIES})

# Create  a main program using the library


#list(APPEND RMODULES bscint_adc_module.cxx) # NM
list(APPEND RMODULES src/bsc_adc_module)     # AC
#list(APPEND RMODULES tdc_module.o
list(APPEND RMODULES src/bsc_tdc_module)	# NM
#list(APPEND RMODULES fast_deconv_module.o match_module.o reco_module.o)
#list(APPEND RMODULES deconv_module.o match_module.o reco_module.o)
list(APPEND RMODULES src/deconv_aw_module src/deconv_pad_module src/match_module src/reco_module)
#list(APPEND RMODULES adc_module.o pwb_module.o feam_module.o tpc_tree_module.o)
list(APPEND RMODULES src/histo_module)
list(APPEND RMODULES src/phspectrum_module)
list(APPEND RMODULES src/calib_module)
list(APPEND RMODULES src/display_module)
list(APPEND RMODULES src/official_time_module)
list(APPEND RMODULES src/AnalysisReport_module)


message(${ROOT_INCLUDE_DIRS})
#RMODULES := $(patsubst %.o,$(OBJDIR)/%.o,$(RMODULES))
add_executable(basicApp $ENV{ROOTANASYS}/manalyzer/manalyzer_main  ${RMODULES})
#target_link_libraries(basicApp ${ROOT_LIBRARIES} agana  rootana AGTPC aged )
#target_include_directories(basicApp PUBLIC $ENV{AGRELEASE}/analib/include ${ROOT_INCLUDE_DIRS})

target_include_directories(basicApp PUBLIC include)
target_include_directories(basicApp PUBLIC $ENV{AGRELEASE}/recolib/include)
target_include_directories(basicApp PUBLIC $ENV{AGRELEASE}/analib/include)
target_include_directories(basicApp PUBLIC $ENV{AGRELEASE}/aged/include)
target_include_directories(basicApp PUBLIC ${ROOT_INCLUDE_DIRS})
link_directories(basicApp analib)
target_link_libraries(basicApp ${ROOT_LIBRARIES} ${ROOTANALIBS} ${X11_LIBRARIES} Xt Xm Minuit2 rootana agana AGTPC aged)
