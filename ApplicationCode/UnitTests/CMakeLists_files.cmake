
CONFIGURE_FILE( ${CMAKE_CURRENT_LIST_DIR}/RiaTestDataDirectory.h.cmake
                ${CMAKE_BINARY_DIR}/Generated/RiaTestDataDirectory.h
)

set (SOURCE_GROUP_HEADER_FILES
)

set (SOURCE_GROUP_SOURCE_FILES
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
${CMAKE_CURRENT_LIST_DIR}/RifEclipseSummaryAddress-Test.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaTimeHistoryCurveTools-Test.cpp
${CMAKE_CURRENT_LIST_DIR}/SolveSpaceSolver-Test.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaPolyArcLineSampler-Test.cpp
${CMAKE_CURRENT_LIST_DIR}/RifEclipseDataTableFormatter-Test.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaWeightedMean-Test.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaWeightedGeometricMeanCalculator-Test.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaWeightedHarmonicMeanCalculator-Test.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "UnitTests" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
