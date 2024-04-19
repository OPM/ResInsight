if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-deprecated-copy")
endif()

project (clipper)

set(project_source_files
    clipper.cpp
)

add_library(${PROJECT_NAME} 
  STATIC
  ${project_source_files}
)

target_include_directories(${PROJECT_NAME}
  PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
)

source_group("" FILES ${PROJECT_FILES})
