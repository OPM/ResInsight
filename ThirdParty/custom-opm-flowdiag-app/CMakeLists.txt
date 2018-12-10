cmake_minimum_required (VERSION 2.8.12)

project (custom-opm-flowdiag-app)

include_directories(
    ../custom-opm-flowdiagnostics/opm-flowdiagnostics
    opm-flowdiagnostics-applications
)

include (opm-flowdiagnostics-applications/CMakeLists_files.cmake)

set(project_source_files
    ${MAIN_SOURCE_FILES} 
    ${PUBLIC_HEADER_FILES}
)

foreach (file ${project_source_files} )
    list(APPEND project_source_files_complete_path1 "opm-flowdiagnostics-applications/${file}" )
endforeach()

add_definitions(-DHAVE_ERT_ECL_TYPE_H)

add_library(custom-opm-flowdiag-app
    ${project_source_files_complete_path1} 
)

target_link_libraries(custom-opm-flowdiag-app 
  ecl
  boost-Subset
)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4190 /wd4267")
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/opm-flowdiagnostics-applications
)
