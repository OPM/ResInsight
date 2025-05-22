cmake_minimum_required(VERSION 3.22.0)
project(surfio_lib LANGUAGES CXX)

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

