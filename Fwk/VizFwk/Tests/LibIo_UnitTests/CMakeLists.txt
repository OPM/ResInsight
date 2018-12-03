cmake_minimum_required(VERSION 2.8)

project(LibIo_UnitTests)

# Compile flags should already be setup by caller


include_directories(${LibCore_SOURCE_DIR})
include_directories(${LibIo_SOURCE_DIR})
include_directories(${VizFramework_SOURCE_DIR}/ThirdParty)

set(CEE_LIBS LibIo LibCore)


set(CEE_SOURCE_FILES
cvfFile-Test.cpp
cvfPropertyXmlSerialization-Test.cpp
cvfXml-Test.cpp
../../ThirdParty/gtest/gtest-all.cpp
LibIo_UnitTests.cpp
)

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
endif()

add_executable(${PROJECT_NAME} ${CEE_SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} ${CEE_LIBS} ${CEE_PLATFORM_LIBS} )

