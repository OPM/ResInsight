cmake_minimum_required(VERSION 3.22.0)
project(surfio_lib LANGUAGES CXX)

# mdspan is part of C++23. kokkos/mdspan is the reference implementation for the standard proposal.
# This can be removed when all needed compilers/libs support it.
FetchContent_Declare(
  mdspan
  GIT_REPOSITORY https://github.com/kokkos/mdspan.git
  GIT_TAG 9ceface91483775a6c74d06ebf717bbb2768452f # mdspan-0.6.0
)
FetchContent_MakeAvailable(mdspan)

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
  PRIVATE $<BUILD_INTERFACE:${mdspan_SOURCE_DIR}/include/>
  PRIVATE ../mio
)

