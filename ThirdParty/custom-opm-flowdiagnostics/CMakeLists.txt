cmake_minimum_required (VERSION 2.8)

project (custom-opm-flowdiagnostics)

include_directories(
    opm-flowdiagnostics
)

include (opm-flowdiagnostics/CMakeLists_files.cmake)

set(project_source_files
    ${MAIN_SOURCE_FILES} 
    ${PUBLIC_HEADER_FILES}
)

foreach (file ${project_source_files} )
    list(APPEND project_source_files_complete_path "opm-flowdiagnostics/${file}" )
endforeach()

add_library(${PROJECT_NAME} 
    ${project_source_files_complete_path} 
)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4267")
endif()
