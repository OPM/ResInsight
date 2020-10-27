cmake_minimum_required(VERSION 2.8)

project(OdbReader_UnitTests)

set(RI_VIZ_FWK_ROOT
    ../../../Fwk/VizFwk
    CACHE PATH "Path to VizFwk")
set(RI_GTEST_ROOT
    ../../../ThirdParty
    CACHE PATH "Path to folder containing gtest folder")
set(RI_TEST_FILE
    ""
    CACHE FILEPATH "Path to test file")

include(${RI_VIZ_FWK_ROOT}/CMake/Utils/ceeDetermineCompilerFlags.cmake)

add_subdirectory(${RI_VIZ_FWK_ROOT}/LibCore buildVizFwk)
add_subdirectory(../../ResultStatisticsCache buildResultStatisticsCache)
add_subdirectory(../OdbReader buildOdbReader)
add_subdirectory(../GeoMechDataModel buildGeoMechDataModel)

add_definitions(-DTEST_FILE="${RI_TEST_FILE}")

include_directories(${RI_VIZ_FWK_ROOT}/LibCore)
include_directories(../../ResultStatisticsCache)
include_directories(../../ReservoirDataModel)
include_directories(../OdbReader)
include_directories(../GeoMechDataModel)
include_directories(${RI_GTEST_ROOT})

set(UNIT_TEST_CPP_SOURCES main.cpp RifOdbReader-Test.cpp
                          ${RI_GTEST_ROOT}/gtest/gtest-all.cc)

add_executable(${PROJECT_NAME} ${UNIT_TEST_CPP_SOURCES})
target_link_libraries(${PROJECT_NAME} RifOdbReader)

include(../OdbReader/OdbSetup.cmake)
