cmake_minimum_required(VERSION 3.12)

project(cafHexInterpolator)

set(PROJECT_FILES cafHexInterpolator.h)

add_library(${PROJECT_NAME} INTERFACE ${PROJECT_FILES} )

target_include_directories(${PROJECT_NAME} INTERFACE ${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME} LibCore)

source_group("" FILES ${PROJECT_FILES})
