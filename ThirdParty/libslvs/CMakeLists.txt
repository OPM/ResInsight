cmake_minimum_required(VERSION 2.8.12)

# Platform utilities

if(MSVC)
	add_definitions(-D_SCL_SECURE_NO_WARNINGS -D_CRT_SECURE_NO_WARNINGS)
endif()

option(SLVS_BUILD_SHARED "Build SolveSpaceLib as a .dll or .so" ON)

if (${SLVS_BUILD_SHARED})
	set(SLVS_LIB_TYPE "SHARED")
	set(SLVS_SHARED_LIB_DEFINE "-DSLVS_LIB_SHARED")
else()
	set(SLVS_LIB_TYPE "STATIC")
endif()

add_library(libslvs ${SLVS_LIB_TYPE}
    constrainteq.cpp
	dsc.h
    entity.cpp
	expr.h
    expr.cpp
	polygon.h
	resource.h
	sketch.h
    solvespace.h
    system.cpp
	util.cpp
    platform/platform.h
    platform/unixutil.cpp
	include/slvs.h
	srf/surface.h
    lib.cpp
	include/SolveSpaceSystem.h
	SolveSpaceSystem.cpp)

if (${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU")
    target_compile_options(libslvs
        PRIVATE -Wno-missing-field-initializers
		PUBLIC -std=c++11)
endif()

target_compile_definitions(libslvs
	PUBLIC ${SLVS_SHARED_LIB_DEFINE}
	PRIVATE -DLIBRARY)

target_include_directories(libslvs
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}/include 
    PRIVATE ${CMAKE_CURRENT_LIST_DIR} )

set_target_properties(libslvs PROPERTIES
    PUBLIC_HEADER ${CMAKE_SOURCE_DIR}/include/slvs.h
    VERSION ${solvespace_VERSION_MAJOR}.${solvespace_VERSION_MINOR}
    SOVERSION 1)

