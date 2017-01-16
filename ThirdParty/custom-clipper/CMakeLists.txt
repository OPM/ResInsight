cmake_minimum_required (VERSION 2.8)

project (custom-clipper)

include_directories(
    clipper
)

#include (opm-flowdiagnostics/CMakeLists_files.cmake)

set(project_source_files
    clipper.cpp
)

foreach (file ${project_source_files} )
    list(APPEND project_source_files_complete_path "clipper/${file}" )
endforeach()

add_library(${PROJECT_NAME} 
    ${project_source_files_complete_path} 
)

