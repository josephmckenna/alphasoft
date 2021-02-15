# Modified from https://iscinumpy.gitlab.io/post/cry-cosmic-ray-generator-in-cmake/

project(agdaq)
# CRY
include(ExternalProject)
ExternalProject_Add(
    CRY-1.7
    URL http://nuclear.llnl.gov/simulation/cry_v1.7.tar.gz
    URL_MD5 85f240bebe81fe0b257e92ef1d390a83
    CONFIGURE_COMMAND ""
    INSTALL_COMMAND ""
    BUILD_IN_SOURCE 1
    TEST_AFTER_INSTALL 1
)

ExternalProject_Get_Property(CRY-1.7 source_dir)
set(CRY_INCLUDE_DIRS "${source_dir}/src")
file(GLOB Sources "${source_dir}/*.cxx")
set(CRY_LIBRARY_DIR "${source_dir}/lib")
set(CRY_LIBRARIES "${CRY_LIBRARY_DIR}/libCRY.a")
include_directories(${CRY_INCLUDE_DIRS})
link_directories(${CRY_LIBRARY_DIR})
set(LINK_LIBS ${CRY_LIBRARIES} ${LINK_LIBS})
