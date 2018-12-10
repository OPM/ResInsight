
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAddWellLogToPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogCurveExtractionFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogRftCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewRftPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteRftPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewPltPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeletePltPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogFileCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogPlotFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogPlotTrackFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellLogPlotCurveFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicWellLogsImportFileFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellLogPlotTrackFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellLogPlotTrackFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteWellLogCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteWellLogTrackFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteWellLogPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicChangeDataSourceFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicAsciiExportWellLogPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellLogFileCloseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicMoveWellLogFilesFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicAdd3dWellLogCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicAdd3dWellLogFileCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicAdd3dWellLogRftCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/Ric3dWellLogCurveDeleteFeature.h
${CMAKE_CURRENT_LIST_DIR}/Ric3dWellLogCurvePickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RicNewWellBoreStabilityPlotFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAddWellLogToPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogCurveExtractionFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogRftCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewRftPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteRftPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewPltPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeletePltPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogFileCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogPlotFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellLogPlotTrackFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellLogPlotCurveFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellLogsImportFileFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDeleteWellLogPlotTrackFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellLogPlotTrackFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteWellLogCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteWellLogTrackFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteWellLogPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicChangeDataSourceFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicAsciiExportWellLogPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellLogFileCloseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicMoveWellLogFilesFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicAdd3dWellLogCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicAdd3dWellLogFileCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicAdd3dWellLogRftCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/Ric3dWellLogCurveDeleteFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/Ric3dWellLogCurvePickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewWellBoreStabilityPlotFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\WellLog" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
