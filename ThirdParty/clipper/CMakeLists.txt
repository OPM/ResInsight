cmake_minimum_required (VERSION 2.8)

project (clipper)

set(project_source_files
    clipper.cpp
)

add_library(${PROJECT_NAME} 
  STATIC
  ${project_source_files}
)

