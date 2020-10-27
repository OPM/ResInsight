cmake_minimum_required(VERSION 3.12)

project(cafHexInterpolator)

set(PROJECT_FILES cafHexInterpolator.h)

add_library(${PROJECT_NAME} ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} LibCore)

source_group("" FILES ${PROJECT_FILES})
