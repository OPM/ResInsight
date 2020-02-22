cmake_minimum_required (VERSION 2.8)

project ( well_UnitTests )

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message("Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/../..
)

set( PROJECT_FILES

	well_UnitTests.cpp
	../../gtest/gtest-all.cc

	wellBasicTest.cpp
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

source_group("" FILES ${PROJECT_FILES})

target_link_libraries ( ${PROJECT_NAME}
    NRLib
)

