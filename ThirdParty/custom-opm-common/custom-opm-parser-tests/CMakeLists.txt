cmake_minimum_required (VERSION 2.8)

project ( opm-parser-tests )

if (MSVC AND (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 19.11))
  # VS 2017 : Disable warnings from from gtest code, using deprecated code related to TR1 
  add_definitions(-D_SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING)
  message(STATUS "Add flag to disable warings from gtest -  _SILENCE_TR1_NAMESPACE_DEPRECATION_WARNING")
endif()

CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/OpmTestDataDirectory.h.cmake
                ${CMAKE_BINARY_DIR}/Generated/OpmTestDataDirectory.h
)

include_directories (
  ${CMAKE_CURRENT_SOURCE_DIR}/../..
  ${CMAKE_BINARY_DIR}/Generated
#    ${CMAKE_CURRENT_SOURCE_DIR}/../opm-parser
    
#	${CMAKE_CURRENT_SOURCE_DIR}/../../custom-opm-common/opm-common

 #   ${ERT_INCLUDE_DIRS}
    
  #  ${Boost_INCLUDE_DIRS}
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
    custom-opm-common
)

# Add dependency of Shlwapi.lib for Windows platforms
if (CMAKE_CXX_COMPILER_ID STREQUAL "MSVC")
  target_link_libraries(${PROJECT_NAME} Shlwapi)
endif()