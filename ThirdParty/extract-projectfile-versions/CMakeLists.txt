cmake_minimum_required(VERSION 3.15)
project(extract-projectfile-versions)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC ON)

find_package(Qt6 COMPONENTS Core Sql REQUIRED)

add_executable(extract-projectfile-versions main.cpp)

target_link_libraries(extract-projectfile-versions Qt6::Core Qt6::Sql)

if(MSVC)
  add_custom_command(
    TARGET extract-projectfile-versions
    POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_RUNTIME_DLLS:extract-projectfile-versions>
            $<TARGET_FILE_DIR:extract-projectfile-versions>
    COMMAND_EXPAND_LISTS
  )
endif(MSVC)
