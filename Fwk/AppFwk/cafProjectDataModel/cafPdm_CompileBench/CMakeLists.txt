cmake_minimum_required(VERSION 3.15)

project(cafPdm_CompileBench)

find_package(
  Qt6
  COMPONENTS
  REQUIRED Core Widgets
)
qt_standard_project_setup()

set(PROJECT_FILES
    bench_basic_fields.cpp bench_enum_fields.cpp bench_container_fields.cpp
    bench_filepath_fields.cpp bench_mixed_fields.cpp
)

# OBJECT library: compiles each file individually without linking. This isolates
# the compile-time cost per translation unit.
add_library(${PROJECT_NAME} OBJECT ${PROJECT_FILES})

# Disable unity build for this target — each file must compile independently
set_target_properties(${PROJECT_NAME} PROPERTIES UNITY_BUILD OFF)

target_link_libraries(
  ${PROJECT_NAME} PRIVATE cafProjectDataModel Qt6::Core Qt6::Widgets
)
