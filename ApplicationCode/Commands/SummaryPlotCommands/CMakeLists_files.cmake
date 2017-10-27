
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()


set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicNewSummaryPlotFeature.h
${CEE_CURRENT_LIST_DIR}RicNewSummaryCurveFeature.h
${CEE_CURRENT_LIST_DIR}RicNewSummaryCurveFilterFeature.h
${CEE_CURRENT_LIST_DIR}RicPasteAsciiDataToSummaryPlotFeature.h
${CEE_CURRENT_LIST_DIR}RicPasteAsciiDataToSummaryPlotFeatureUi.h
${CEE_CURRENT_LIST_DIR}RicViewZoomAllFeature.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveSwitchAxisFeature.h
${CEE_CURRENT_LIST_DIR}RicPasteSummaryPlotFeature.h
${CEE_CURRENT_LIST_DIR}RicPasteSummaryCurveFeature.h
${CEE_CURRENT_LIST_DIR}RicPasteSummaryCaseFeature.h
${CEE_CURRENT_LIST_DIR}RicAsciiExportSummaryPlotFeature.h
${CEE_CURRENT_LIST_DIR}RicNewGridTimeHistoryCurveFeature.h
${CEE_CURRENT_LIST_DIR}RicSelectSummaryPlotUI.h
${CEE_CURRENT_LIST_DIR}RicPasteTimeHistoryCurveFeature.h
${CEE_CURRENT_LIST_DIR}RicPasteAsciiDataCurveFeature.h
${CEE_CURRENT_LIST_DIR}RicEditSummaryPlotFeature.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreator.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreatorSplitterUi.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreatorDialog.h
${CEE_CURRENT_LIST_DIR}RicShowSummaryCurveCalculatorFeature.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCalculatorDialog.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCalculatorEditor.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCalculator.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicNewSummaryPlotFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewSummaryCurveFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewSummaryCurveFilterFeature.cpp
${CEE_CURRENT_LIST_DIR}RicPasteAsciiDataToSummaryPlotFeature.cpp
${CEE_CURRENT_LIST_DIR}RicPasteAsciiDataToSummaryPlotFeatureUi.cpp
${CEE_CURRENT_LIST_DIR}RicViewZoomAllFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSummaryCurveSwitchAxisFeature.cpp
${CEE_CURRENT_LIST_DIR}RicPasteSummaryPlotFeature.cpp
${CEE_CURRENT_LIST_DIR}RicPasteSummaryCurveFeature.cpp
${CEE_CURRENT_LIST_DIR}RicPasteSummaryCaseFeature.cpp
${CEE_CURRENT_LIST_DIR}RicAsciiExportSummaryPlotFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewGridTimeHistoryCurveFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSelectSummaryPlotUI.cpp
${CEE_CURRENT_LIST_DIR}RicPasteTimeHistoryCurveFeature.cpp
${CEE_CURRENT_LIST_DIR}RicPasteAsciiDataCurveFeature.cpp
${CEE_CURRENT_LIST_DIR}RicEditSummaryPlotFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreator.cpp
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreatorSplitterUi.cpp
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreatorDialog.cpp
${CEE_CURRENT_LIST_DIR}RicShowSummaryCurveCalculatorFeature.cpp
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCalculatorDialog.cpp
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCalculatorEditor.cpp
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCalculator.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

list(APPEND QT_MOC_HEADERS
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreatorSplitterUi.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCreatorDialog.h
${CEE_CURRENT_LIST_DIR}RicSummaryCurveCalculatorEditor.h
)


source_group( "CommandFeature\\SummaryPlot" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
