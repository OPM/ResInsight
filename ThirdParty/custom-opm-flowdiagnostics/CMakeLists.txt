project (custom-opm-flowdiagnostics)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations")
endif()

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
else()
    set_target_properties(custom-opm-flowdiagnostics PROPERTIES COMPILE_FLAGS "-Wno-deprecated -Wno-deprecated-declarations")
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/opm-flowdiagnostics
)
