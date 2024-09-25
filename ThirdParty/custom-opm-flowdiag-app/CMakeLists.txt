project (custom-opm-flowdiag-app)

if (CMAKE_COMPILER_IS_GNUCXX)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations")
endif()

if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated -Wno-deprecated-declarations -Wno-delete-abstract-non-virtual-dtor -Wno-deprecated-copy")
endif()

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

find_package(Boost REQUIRED filesystem system)

target_link_libraries(custom-opm-flowdiag-app 
  resdata
  Boost::filesystem
)

if (MSVC)
    set_target_properties(${PROJECT_NAME} PROPERTIES COMPILE_FLAGS "/wd4190 /wd4267")
else()
    set_target_properties(custom-opm-flowdiag-app PROPERTIES COMPILE_FLAGS "-Wno-deprecated -Wno-deprecated-declarations")
endif()

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/opm-flowdiagnostics-applications
)
