configure_file(
  ${CMAKE_CURRENT_LIST_DIR}/RiaTestDataDirectory.h.cmake
  ${CMAKE_BINARY_DIR}/Generated/RiaTestDataDirectory.h
)

set(SOURCE_GROUP_HEADER_FILES)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/cvfGeometryTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Ert-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifcCommandCore-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifEclipseInputFileTools-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseOutput-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifReaderEclipseSummary-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigActiveCellInfo-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigReservoir-Test.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/RiaProjectFileVersionTools-Test.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveAnalyzer-Test.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/RigWellPathGeometryExporter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifStimPlanModelDeviationFrkExporter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifSummaryDataReader-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSlice2D-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSurfaceResampler-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RigSurfaceStatisticsCalculator-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/StructGridInterface-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/opm-summary-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/GrdeclImporter-Test.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RifEclipseTextFileReader-Test.cpp
)

if(RESINSIGHT_ENABLE_GRPC)
  list(APPEND GPRC_UNIT_TEST_SOURCE_FILES
       ${CMAKE_CURRENT_LIST_DIR}/RiaGrpcInterface-Test.cpp
  )
  list(APPEND SOURCE_GROUP_SOURCE_FILES ${GRPC_UNIT_TEST_SOURCE_FILES})
endif(RESINSIGHT_ENABLE_GRPC)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

source_group(
  "UnitTests" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
                    ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
