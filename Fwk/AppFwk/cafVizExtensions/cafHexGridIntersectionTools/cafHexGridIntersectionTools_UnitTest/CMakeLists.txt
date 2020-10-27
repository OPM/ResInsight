cmake_minimum_required(VERSION 2.8.12)

project(cafHexGridIntersectionTools_UnitTests)

if(MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code
  # related to TR1
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message(
    STATUS
      "Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING"
  )
endif()

set(VIZ_MODULES_FOLDER_NAME ../../../../../Fwk/VizFwk)
add_subdirectory(${VIZ_MODULES_FOLDER_NAME}/LibCore LibCoreDir})

include_directories(${CMAKE_CURRENT_SOURCE_DIR} # required for gtest-all.cpp
)

set(PROJECT_FILES
    cafHexGridIntersectionToolsBasicTests.cpp
    cafHexGridIntersectionTools_UnitTests_main.cpp
    ../cafHexGridIntersectionTools.cpp ../cafHexGridIntersectionTools.h)

# add the executable
add_executable(${PROJECT_NAME} ${PROJECT_FILES} gtest/gtest-all.cpp)

target_link_libraries(${PROJECT_NAME} LibCore)

source_group("" FILES ${PROJECT_FILES})
