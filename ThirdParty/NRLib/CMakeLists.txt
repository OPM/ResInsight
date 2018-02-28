cmake_minimum_required (VERSION 2.8.12)

project (NRLib)

include_directories(
       ${CMAKE_CURRENT_SOURCE_DIR}
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
       STATIC
       ${NRLIB_IOTOOLS_SRC}
       ${NRLIB_WELL_SRC}
)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4996")
endif()

add_subdirectory(well_UnitTests)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/nrlib/well
)

target_link_libraries ( ${PROJECT_NAME}
  boost-Subset
)
