cmake_minimum_required (VERSION 2.8)

project ( opm-parser-tests )

# Languages and global compiler settings
if(CMAKE_VERSION VERSION_LESS 3.8)
  message(WARNING "CMake version does not support c++17, guessing -std=c++17")
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++17")
else()
  set(CMAKE_CXX_STANDARD 17)
  set(CMAKE_CXX_STANDARD_REQUIRED ON)
  set(CMAKE_CXX_EXTENSIONS OFF)
endif()

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
if (MSVC)
  target_link_libraries(${PROJECT_NAME} Shlwapi)
endif()