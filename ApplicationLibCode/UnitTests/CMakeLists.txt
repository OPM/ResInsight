# ResInsight unit tests

if(MSVC)
  # TARGET_RUNTIME_DLLS requires 3.21
  cmake_minimum_required(VERSION 3.21)
endif(MSVC)

set(SOURCE_UNITTEST_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cvfGeometryTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Ert-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportEclipseSectorModelFeature-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigBoundingBoxIjk-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifcCommandCore-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifEclipseInputFileTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifOpmFlowDeckFile-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseOutput-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseSummary-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigActiveCellInfo-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseCaseDataTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigEclipseResultTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigReservoir-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigResdataGridConverter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigGridExportAdapter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigStatisticsMath-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathIntersectionTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurveImpl-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivPipeGeometryGenerator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RivTernaryScalarMapper-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ScalarMapper-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/WellPathAsciiFileReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/opm-flowdiagnostics-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigTofAccumulatedPhaseFractionsCalculator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HDF5FileReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/HDF5FileWriter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigCellGeometryTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigHexIntersectionTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ObservedDataParser-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/EclipseRftReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExpressionParser-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiuSummaryVectorDescriptionMap-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/FixedWidthDataParser-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigTimeCurveHistoryMerger-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/ListKeywordsForObjectsAndFields-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaProjectFileTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifElementPropertyTableReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRelocatePath-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigTransmissibilityCondenser-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaEclipseUnitTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaTextFileCompare-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifCaseRealizationParametersReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogExtractor-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathGeometryTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifEclipseSummaryAddress-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaTimeHistoryCurveTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/SolveSpaceSolver-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaPolyArcLineSampler-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifTextDataTableFormatter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaWeightedMean-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaMedianCalculator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaWeightedGeometricMeanCalculator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaWeightedHarmonicMeanCalculator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaCellDividingTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaFilePathTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Intersect-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifPerforationIntervalReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPathCompletions-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCaseCollection-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifActiveCellsReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifCsvDataTableFormatter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryAddressAnalyzer-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaStdStringTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaInterpolationTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifWellMeasurementReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaDateStringParser-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigHexGradientTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifSurfaceImporter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifColorLegendData-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifRoffReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifElasticPropertiesReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaStatisticsTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifStimPlanXmlReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifThermalFractureReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathGeometryExporter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifStimPlanModelDeviationFrkExporter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifSummaryDataReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSlice2D-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigConvexHull-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSurfaceResampler-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSurfaceStatisticsCalculator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StructGridInterface-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/opm-summary-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifEclipseTextFileReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryStringTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaVariableMapper-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifPressureDepthTextFileReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifRevealCsvSectionSummaryReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifRevealCsvSummaryReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifStimPlanCsvSummaryReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaEnsembleNameTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigDeclineCurveCalculator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifReaderFmuRft-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryRegressionAnalysisCurve-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCalculatedCurve-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigWellLogCurveData-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaNumericalTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/opm-import-well-data-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifInpExportTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifGridCalculationIO-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifSummaryCalculationIO-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEmReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifPolygonReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifParquetReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifOsduWellPathReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigVfpTables-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaResultName-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigPolygonTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifVtkSurfaceImporter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaKeyValueStore-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaEnsembleImportTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifSurfio-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifRmsWellPathReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RiaAngleUtils-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/cafVecIjk-Test.cpp
)

if(RESINSIGHT_ENABLE_GRPC)
  list(APPEND SOURCE_UNITTEST_FILES
       ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcInterface-Test.cpp
  )
endif(RESINSIGHT_ENABLE_GRPC)

# Obsolete test
if(RESINSIGHT_USE_ODB_API_OBSOLETE)
  list(APPEND SOURCE_UNITTEST_FILES
       ${CMAKE_CURRENT_LIST_DIR}/RifOdbReader-Test.cpp
  )
endif(RESINSIGHT_USE_ODB_API_OBSOLETE)

include(FetchContent)
FetchContent_Declare(
  googletest
  GIT_REPOSITORY https://github.com/google/googletest.git
  GIT_TAG release-1.11.0
)

set(BUILD_GMOCK
    OFF
    CACHE BOOL "" FORCE
)
set(BUILD_GTEST
    ON
    CACHE BOOL "" FORCE
)
set(INSTALL_GTEST
    OFF
    CACHE BOOL "" FORCE
)
FetchContent_MakeAvailable(googletest)

# ##############################################################################
# Copy required Dll/.so to output folder
# ##############################################################################

# create an empty library target that will be used to copy files to the build
# folder
add_library(ResInsightDummyTestTarget EXCLUDE_FROM_ALL empty.cpp)
set_property(
  TARGET ResInsightDummyTestTarget PROPERTY FOLDER "FileCopyTargetsTest"
)

# create a custom target that copies the files to the build folder
foreach(riFileName ${RI_FILENAMES})
  list(
    APPEND
    copyCommands
    COMMAND
    ${CMAKE_COMMAND}
    -E
    copy_if_different
    ${riFileName}
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
  )
endforeach()
add_custom_target(PreBuildFileCopyTest ${copyCommands})
set_property(TARGET PreBuildFileCopyTest PROPERTY FOLDER "FileCopyTargetsTest")

qt_add_executable(ResInsight-tests ${SOURCE_UNITTEST_FILES} main.cpp)

# Make ResInsight-tests depend on the prebuild target.
add_dependencies(ResInsight-tests PreBuildFileCopyTest)

configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/RiaTestDataDirectory.h.cmake
  ${CMAKE_BINARY_DIR}/Generated/RiaTestDataDirectory.h
)

target_include_directories(
  ResInsight-tests
  PUBLIC ${CMAKE_BINARY_DIR}/Generated
         "$<TARGET_PROPERTY:ApplicationLibCode,PUBLIC_INCLUDE_DIRECTORIES>"
         ${RI_PRIVATE_INCLUDES} ${PROJECT_SOURCE_DIR}/Commands
)

target_compile_features(ResInsight-tests PRIVATE cxx_std_20)

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)

if(${CMAKE_SYSTEM_NAME} MATCHES "Linux")
  list(APPEND THIRD_PARTY_LIBRARIES rt atomic)
endif()

set(LINK_LIBRARIES
    ${THIRD_PARTY_LIBRARIES}
    ${OPENGL_LIBRARIES}
    ${QT_LIBRARIES}
    ${OPM_LIBRARIES}
    ${APP_FWK_LIBRARIES}
    ${VIZ_FWK_LIBRARIES}
    ApplicationLibCode
    Commands
    RigGeoMechDataModel
    RifGeoMechFileInterface
)

if(RESINSIGHT_ENABLE_GRPC)
  list(APPEND LINK_LIBRARIES GrpcInterface)
endif()

if(RESINSIGHT_USE_ODB_API)
  list(APPEND LINK_LIBRARIES RifOdbReader)
endif()

target_link_libraries(ResInsight-tests PUBLIC ${LINK_LIBRARIES} GTest::gtest)

if(MSVC)
  add_custom_command(
    TARGET ResInsight-tests
    POST_BUILD
    COMMAND
      ${CMAKE_COMMAND} -E copy_if_different
      $<TARGET_RUNTIME_DLLS:ResInsight-tests>
      $<TARGET_FILE_DIR:ResInsight-tests>
    COMMAND_EXPAND_LISTS
  )

  set_target_properties(ResInsight-tests PROPERTIES COMPILE_FLAGS "/wd4190")
endif(MSVC)

add_test(NAME ResInsight-tests COMMAND ResInsight-tests)
