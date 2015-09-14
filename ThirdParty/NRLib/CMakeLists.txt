cmake_minimum_required (VERSION 2.8)

project (NRLib)

include_directories(
       ${CMAKE_CURRENT_SOURCE_DIR}
       ${CMAKE_CURRENT_SOURCE_DIR}/boost
       ${CMAKE_CURRENT_SOURCE_DIR}/nrlib
       ${CMAKE_CURRENT_SOURCE_DIR}/nrlib/well
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
       boost/filesystem/operations.cpp
       boost/filesystem/path.cpp
       boost/filesystem/portability.cpp
       boost/system/error_code.cpp
       ${NRLIB_IOTOOLS_SRC}
       ${NRLIB_WELL_SRC}
)

add_subdirectory(well_UnitTests)

