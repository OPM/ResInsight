
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryCrossPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryCrossPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryCrossPlotCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteAsciiDataToSummaryPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteAsciiDataToSummaryPlotFeatureUi.h
${CMAKE_CURRENT_LIST_DIR}/RicViewZoomAllFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveSwitchAxisFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCrossPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCrossPlotCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCaseFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicAsciiExportSummaryPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewGridTimeHistoryCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSelectSummaryPlotUI.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteTimeHistoryCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteAsciiDataCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEditSummaryPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEditSummaryCrossPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreator.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreatorSplitterUi.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreatorDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicShowSummaryCurveCalculatorFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicEditSummaryCurveCalculationFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculatorDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculatorEditor.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryCrossPlotCurveFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryEnsembleCurveSetFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicPasteEnsembleCurveSetFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryCrossPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryCrossPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicDuplicateSummaryCrossPlotCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteAsciiDataToSummaryPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteAsciiDataToSummaryPlotFeatureUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicViewZoomAllFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveSwitchAxisFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCrossPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCrossPlotCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteSummaryCaseFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicAsciiExportSummaryPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewGridTimeHistoryCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSelectSummaryPlotUI.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteTimeHistoryCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteAsciiDataCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEditSummaryPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEditSummaryCrossPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreator.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreatorSplitterUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreatorDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RicShowSummaryCurveCalculatorFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicEditSummaryCurveCalculationFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculatorDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculatorEditor.cpp
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryCrossPlotCurveFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryEnsembleCurveSetFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicPasteEnsembleCurveSetFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

list(APPEND QT_MOC_HEADERS
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreatorSplitterUi.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCreatorDialog.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculatorEditor.h
${CMAKE_CURRENT_LIST_DIR}/RicSummaryCurveCalculatorDialog.h
)

source_group( "CommandFeature\\SummaryPlot" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
