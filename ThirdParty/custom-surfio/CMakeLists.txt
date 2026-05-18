cmake_minimum_required(VERSION 3.22.0)
project(surfio_lib LANGUAGES CXX)

# mdspan is part of C++23. kokkos/mdspan is the reference implementation for the standard proposal.
# This can be removed when all needed compilers/libs support it.
FetchContent_Declare(
  mdspan
  GIT_REPOSITORY https://github.com/kokkos/mdspan.git
  GIT_TAG 9ceface91483775a6c74d06ebf717bbb2768452f # mdspan-0.6.0
)

# Disable install for mdspan by excluding it from INSTALL_ALL
FetchContent_GetProperties(mdspan)
if(NOT mdspan_POPULATED)
  FetchContent_Populate(mdspan)
  add_subdirectory(${mdspan_SOURCE_DIR} ${mdspan_BINARY_DIR} EXCLUDE_FROM_ALL)
endif()

# surfio uses parse_number/parse_number.h which falls back to fast_float
# when std::from_chars for floats is not available. fast_float is provided via vcpkg.
find_package(FastFloat CONFIG REQUIRED)

# surfio_lib
set(SRC_PATH "${CMAKE_CURRENT_LIST_DIR}/surfio/src/lib")
add_library(surfio_lib)
target_sources(
  surfio_lib
  PRIVATE ${SRC_PATH}/irap_import_ascii.cpp
          ${SRC_PATH}/irap_import_binary.cpp ${SRC_PATH}/irap_export_ascii.cpp
          ${SRC_PATH}/irap_export_binary.cpp
          ${SRC_PATH}/mmap_wrapper/mmap_wrapper.cpp
)
target_include_directories(
  surfio_lib
  INTERFACE ${SRC_PATH}/include
  PRIVATE ../mio
)

target_compile_definitions(surfio_lib PRIVATE STD_FROM_CHARS_FLOAT_SUPPORT=0)

target_link_libraries(surfio_lib PUBLIC mdspan PRIVATE FastFloat::fast_float)

