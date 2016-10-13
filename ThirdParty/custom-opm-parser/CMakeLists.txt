cmake_minimum_required (VERSION 2.8)

# -DBOOST_FILESYSTEM_VERSION=3 -DBOOST_TEST_DYN_LINK -DHAVE_CASE_SENSITIVE_FILESYSTEM=1 -DHAVE_REGEX=1 -DOPM_PARSER_DECK_API=1 -Wall -std=c++11 -fopenmp -Wall -g   -O0 -DDEBUG  -ggdb3
 
 if(MSVC)
  add_definitions( "/W3 /D_CRT_SECURE_NO_WARNINGS /wd4996 /wd4244 /wd4267" )
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
else()
  #always set these flags to make opm-parser compile on Linux
  SET( CMAKE_CXX_FLAGS "-pipe -std=c++0x -pedantic -Wall -Wextra -Wformat-nonliteral -Wcast-align -Wpointer-arith -Wmissing-declarations -Wcast-qual -Wshadow -Wwrite-strings -Wchar-subscripts -Wredundant-decls")

  SET( CMAKE_CXX_FLAGS_DEBUG   "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_DEBUG}   -O0 -DDEBUG  -ggdb3")
  SET( CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS} ${CMAKE_CXX_FLAGS_RELEASE} -O3 -DNDEBUG -mtune=native")
endif(MSVC)

project (custom-opm-parser)

include_directories(
    ${Boost_INCLUDE_DIRS}
    ${ERT_INCLUDE_DIRS}
)

include_directories(
    opm-parser
    generated-source/include
)

# TODO: opm-parser should hold a cmake file with source code files only
#include(opm-parser/CMakeLists_files.cmake)
include ( CMakeLists_files.cmake )

add_library(${PROJECT_NAME} 
    STATIC
	${opm_parser_source_files}
)

