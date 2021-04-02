cmake_minimum_required(VERSION 3.12)

project(cafHexInterpolator)

set(PROJECT_FILES cafHexInterpolator.h)

add_library(${PROJECT_NAME} INTERFACE ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})

source_group("" FILES ${PROJECT_FILES})
