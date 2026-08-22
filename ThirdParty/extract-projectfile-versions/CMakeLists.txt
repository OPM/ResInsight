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

# This standalone developer tool must not share the output directory with the
# application. The POST_BUILD step below copies Qt6Core.dll and Qt6Sql.dll next
# to the executable, and the build root has ResInsight.exe running out of it
# during Python code generation with those same DLLs loaded. Overwriting a
# mapped DLL is a sharing violation on Windows, and nothing orders the two build
# edges relative to each other.
set_target_properties(
  extract-projectfile-versions
  PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

if(MSVC)
  add_custom_command(
    TARGET extract-projectfile-versions
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:extract-projectfile-versions>
      $<TARGET_FILE_DIR:extract-projectfile-versions>
    COMMAND_EXPAND_LISTS
  )
endif(MSVC)
