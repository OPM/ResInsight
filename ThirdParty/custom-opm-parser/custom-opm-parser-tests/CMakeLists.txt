cmake_minimum_required (VERSION 2.8)


project ( opm-parser-tests )

include_directories (
	${CMAKE_CURRENT_SOURCE_DIR}/../..
    ${CMAKE_CURRENT_SOURCE_DIR}/../opm-parser
    
	${CMAKE_CURRENT_SOURCE_DIR}/../../custom-opm-common/opm-common

    ${ERT_INCLUDE_DIRS}
    
    ${Boost_INCLUDE_DIRS}
)

set( PROJECT_FILES

	opm-parser_UnitTests.cpp 
	../../gtest/gtest-all.cc

	opm-parser-BasicTest.cpp 
)

# add the executable 
add_executable (${PROJECT_NAME}
	${PROJECT_FILES}
)

source_group("" FILES ${PROJECT_FILES})

target_link_libraries ( ${PROJECT_NAME}
    ${ERT_LIBRARIES}
    custom-opm-parser
    ${Boost_LIBRARIES}
)

