cmake_minimum_required (VERSION 2.8)

project ( well_UnitTests )


include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/..
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

