
set (SOURCE_GROUP_HEADER_FILES
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
${CMAKE_CURRENT_LIST_DIR}/RimWellLogCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimWellLogFile.h
${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileChannel.h
${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimWellLogRftCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimWellLogWbsCurve.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogTrack.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogFile.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileChannel.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogRftCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogWbsCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogPlotNameConfig.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogExtractionCurveNameConfig.cpp
${CMAKE_CURRENT_LIST_DIR}/RimWellLogFileCurveNameConfig.cpp
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
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\WellLog" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
