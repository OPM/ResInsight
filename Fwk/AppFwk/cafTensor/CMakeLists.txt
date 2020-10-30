cmake_minimum_required(VERSION 3.12)

project(cafTensor)

set(PROJECT_FILES cafTensor3.cpp cafTensor3.h cafTensor3.inl)

add_library(${PROJECT_NAME} ${PROJECT_FILES})

target_include_directories(${PROJECT_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(${PROJECT_NAME} LibCore)

source_group("" FILES ${PROJECT_FILES})
