cmake_minimum_required (VERSION 2.8)

project ( cafTensor_UnitTests )

set(RI_VIZ_FWK_ROOT ../../../Fwk/VizFwk  CACHE PATH "Path to VizFwk")
set(RI_GTEST_ROOT ..  CACHE PATH "Path to folder containing gtest folder")
set(RI_SRC_ROOT ../../cafTensor  CACHE PATH "Path to the code to test")
set(RI_TEST_FILE ""  CACHE FILEPATH "Path to test file")

include(${RI_VIZ_FWK_ROOT}/CMake/Utils/ceeDetermineCompilerFlags.cmake)

add_subdirectory(${RI_VIZ_FWK_ROOT}/LibCore buildVizFwk)

add_definitions( -DTEST_FILE="${RI_TEST_FILE}")

include_directories(${RI_VIZ_FWK_ROOT}/LibCore)
include_directories(${RI_GTEST_ROOT})
include_directories(${RI_SRC_ROOT})

set( UNIT_TEST_CPP_SOURCES
    main.cpp
   	cafTensor_UnitTests.cpp
	${RI_SRC_ROOT}/cafTensor3.cpp
	${RI_SRC_ROOT}/cafTensor3.h
	${RI_SRC_ROOT}/cafTensor3.inl
	
	${RI_GTEST_ROOT}/gtest/gtest-all.cpp
)

add_executable( ${PROJECT_NAME} ${UNIT_TEST_CPP_SOURCES} )
target_link_libraries( ${PROJECT_NAME} LibCore)

