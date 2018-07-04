
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RiaArgumentParser.h
${CMAKE_CURRENT_LIST_DIR}/RiaDateStringParser.h
${CMAKE_CURRENT_LIST_DIR}/RiaColorTables.h
${CMAKE_CURRENT_LIST_DIR}/RiaColorTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaEclipseUnitTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaImageCompareReporter.h
${CMAKE_CURRENT_LIST_DIR}/RiaImageFileCompare.h
${CMAKE_CURRENT_LIST_DIR}/RiaLogging.h
${CMAKE_CURRENT_LIST_DIR}/RiaProjectModifier.h
${CMAKE_CURRENT_LIST_DIR}/RiaRegressionTest.h
${CMAKE_CURRENT_LIST_DIR}/RiaImportEclipseCaseTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaQDateTimeTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaSummaryTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaWellNameComparer.h
${CMAKE_CURRENT_LIST_DIR}/RiaStdStringTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveAnalyzer.h
${CMAKE_CURRENT_LIST_DIR}/RiaSimWellBranchTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaProjectFileVersionTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaStringEncodingTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaTextStringTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaTextFileCompare.h
${CMAKE_CURRENT_LIST_DIR}/RiaRegressionTestRunner.h
${CMAKE_CURRENT_LIST_DIR}/RiaExtractionTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaFilePathTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaTimeHistoryCurveMerger.h
${CMAKE_CURRENT_LIST_DIR}/RiaCurveDataTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaTimeHistoryCurveResampler.h
${CMAKE_CURRENT_LIST_DIR}/RiaStatisticsTools.h
${CMAKE_CURRENT_LIST_DIR}/RiaPolyArcLineSampler.h
${CMAKE_CURRENT_LIST_DIR}/RiaSCurveCalculator.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RiaArgumentParser.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaDateStringParser.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaColorTables.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaColorTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaEclipseUnitTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaImageCompareReporter.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaImageFileCompare.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaLogging.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaProjectModifier.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaRegressionTest.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaImportEclipseCaseTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaQDateTimeTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaSummaryTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaWellNameComparer.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaStdStringTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaSummaryCurveAnalyzer.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaSimWellBranchTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaProjectFileVersionTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaStringEncodingTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaTextStringTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaTextFileCompare.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaRegressionTestRunner.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaExtractionTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaFilePathTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaTimeHistoryCurveMerger.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaCurveDataTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaTimeHistoryCurveResampler.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaStatisticsTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaPolyArcLineSampler.cpp
${CMAKE_CURRENT_LIST_DIR}/RiaSCurveCalculator.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

set (QT_MOC_HEADERS
${QT_MOC_HEADERS}
)


source_group( "Application\\Tools" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
