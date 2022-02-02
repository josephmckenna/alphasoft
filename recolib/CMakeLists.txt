# CMakeLists.txt for basic analysis package.
# It creates a library with dictionary 
cmake_minimum_required(VERSION 3.0 FATAL_ERROR)
project(agtpc)

# Include directories
include_directories(${CMAKE_SOURCE_DIR}/agana)
include_directories(${CMAKE_SOURCE_DIR}/aglib/include)
include_directories(${CMAKE_SOURCE_DIR}/ana/include)
include_directories(${CMAKE_SOURCE_DIR}/recolib/include)


if (BUILD_AG_SIM)
  include_directories(${CMAKE_SOURCE_DIR}/simulation/G4out/include)
endif (BUILD_AG_SIM)
include_directories(include)

file(GLOB LINK_HEADER_FILES RELATIVE ${CMAKE_CURRENT_SOURCE_DIR}/include include/*.hh)
ROOT_GENERATE_DICTIONARY(G__agtpc ${LINK_HEADER_FILES} LINKDEF RecoLinkDef.hh)

# Create a shared library with geneated dictionary
file(GLOB LIBRARY_SOURCES src/*.cxx)
add_library(agtpc SHARED ${LIBRARY_SOURCES} G__agtpc)
target_include_directories(agtpc PUBLIC include)
target_link_libraries(agtpc PUBLIC 
   Matrix # TMatrixBase
   Spectrum # TSpectrum
   MathMore # ROOT::Math::Interpolator
   Core # TObject, _gErrorIgnoreLevel, _gInterpreterMutex
   Graf3d # TPolyLine3D
   Hist # TF1, TFitResultPtr
   MathCore # Math::MinimizerOptions
   RIO # TFile
   Physics # TVector3
   )

if( MINUIT2TRACK )
    target_compile_definitions(agtpc PUBLIC __MINUIT2FIT__)
    message(STATUS "${PROJECT_NAME}: Using Minuit2")
    target_link_libraries(agtpc PUBLIC Minuit2)
else()
    message(STATUS "${PROJECT_NAME}: Using Minuit. Yuk")    
    target_link_libraries(agtpc PUBLIC Minuit)
endif( MINUIT2TRACK )

install(TARGETS agtpc DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libagtpc_rdict.pcm DESTINATION lib)
install(FILES ${PROJECT_BINARY_DIR}/libagtpc.rootmap DESTINATION lib)
file(GLOB HEADER_FILES include/*.hh)
install(FILES ${HEADER_FILES} DESTINATION include)
