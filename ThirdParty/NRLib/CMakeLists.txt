cmake_minimum_required (VERSION 2.8)

ADD_DEFINITIONS("-DBOOST_ALL_NO_LIB")

project (NRLib)

#add_definitions(-DBOOST_FILESYSTEM_VERSION=3)
#find_package(Boost 1.44.0 COMPONENTS filesystem date_time system regex REQUIRED)

include_directories(
       ${CMAKE_CURRENT_SOURCE_DIR}
       ${CMAKE_CURRENT_SOURCE_DIR}/nrlib
       ${CMAKE_CURRENT_SOURCE_DIR}/nrlib/well
       ${Boost_INCLUDE_DIRS}

)

file ( GLOB NRLIB_IOTOOLS_SRC
       nrlib/iotools/*.hpp
       nrlib/iotools/*.cpp
)

file ( GLOB NRLIB_WELL_SRC
       nrlib/well/*.hpp
       nrlib/well/*.cpp
)

add_library( ${PROJECT_NAME}
       ${NRLIB_IOTOOLS_SRC}
       ${NRLIB_WELL_SRC}
)

add_subdirectory(well_UnitTests)

