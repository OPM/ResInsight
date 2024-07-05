set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogFileCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogExtractionCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogRftCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlotNameConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurveNameConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileCurveNameConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogRftCurveNameConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCurveCommonDataSource.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogs.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogsCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogCurveSet.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogStatistics.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogStatisticsCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogTrack.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogLasCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogLasFile.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCsvFile.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLog.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFile.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileUtil.h
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellLogChannel.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogLasFileCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogRftCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogWbsCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimRftTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimRftTopologyCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCurveInfoTextProvider.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCalculatedCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileDataLoader.h
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellLogDataLoader.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogTrack.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogLasFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCsvFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellLog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLog.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileUtil.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogChannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellLogChannel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogLasFileCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogRftCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogWbsCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlotNameConfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurveNameConfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogLasFileCurveNameConfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogRftCurveNameConfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogFileCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogExtractionCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/Rim3dWellLogRftCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCurveCommonDataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogs.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogsCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogCurveSet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogStatistics.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleWellLogStatisticsCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRftTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRftTopologyCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCurveInfoTextProvider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogCalculatedCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileDataLoader.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellLogDataLoader.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND QT_MOC_HEADERS ${CMAKE_CURRENT_LIST_DIR}/RimOsduWellLogDataLoader.h)

source_group(
  "ProjectDataModel\\WellLog"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
