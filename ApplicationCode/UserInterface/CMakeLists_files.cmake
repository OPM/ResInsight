
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RiuCadNavigation.h
${CEE_CURRENT_LIST_DIR}RiuCursors.h
${CEE_CURRENT_LIST_DIR}RiuDragDrop.h
${CEE_CURRENT_LIST_DIR}RiuFemResultTextBuilder.h
${CEE_CURRENT_LIST_DIR}RiuGeoQuestNavigation.h
${CEE_CURRENT_LIST_DIR}RiuInterfaceToViewWindow.h
${CEE_CURRENT_LIST_DIR}RiuLineSegmentQwtPlotCurve.h
${CEE_CURRENT_LIST_DIR}RiuMainPlotWindow.h
${CEE_CURRENT_LIST_DIR}RiuMainWindow.h
${CEE_CURRENT_LIST_DIR}RiuMainWindowBase.h
${CEE_CURRENT_LIST_DIR}RiuMdiSubWindow.h
${CEE_CURRENT_LIST_DIR}RiuMultiCaseImportDialog.h
${CEE_CURRENT_LIST_DIR}RiuProcessMonitor.h
${CEE_CURRENT_LIST_DIR}RiuProjectPropertyView.h
${CEE_CURRENT_LIST_DIR}RiuPropertyViewTabWidget.h
${CEE_CURRENT_LIST_DIR}RiuQwtScalePicker.h
${CEE_CURRENT_LIST_DIR}RiuQwtCurvePointTracker.h
${CEE_CURRENT_LIST_DIR}RiuQwtPlotWheelZoomer.h
${CEE_CURRENT_LIST_DIR}RiuQwtPlotZoomer.h
${CEE_CURRENT_LIST_DIR}RiuRecentFileActionProvider.h
${CEE_CURRENT_LIST_DIR}RiuResultInfoPanel.h
${CEE_CURRENT_LIST_DIR}RiuResultQwtPlot.h
${CEE_CURRENT_LIST_DIR}RiuResultTextBuilder.h
${CEE_CURRENT_LIST_DIR}RiuRmsNavigation.h
${CEE_CURRENT_LIST_DIR}RiuSelectionChangedHandler.h
${CEE_CURRENT_LIST_DIR}RiuSelectionManager.h
${CEE_CURRENT_LIST_DIR}RiuSimpleHistogramWidget.h
${CEE_CURRENT_LIST_DIR}RiuSummaryQwtPlot.h
${CEE_CURRENT_LIST_DIR}RiuTofAccumulatedPhaseFractionsPlot.h
${CEE_CURRENT_LIST_DIR}RiuToolTipMenu.h
${CEE_CURRENT_LIST_DIR}RiuTreeViewEventFilter.h
${CEE_CURRENT_LIST_DIR}RiuViewer.h
${CEE_CURRENT_LIST_DIR}RiuViewerCommands.h
${CEE_CURRENT_LIST_DIR}RiuWellLogPlot.h
${CEE_CURRENT_LIST_DIR}RiuWellLogTrack.h
${CEE_CURRENT_LIST_DIR}RiuGeoMechXfTensorResultAccessor.h
${CEE_CURRENT_LIST_DIR}RiuFemTimeHistoryResultAccessor.h
${CEE_CURRENT_LIST_DIR}RiuEditPerforationCollectionWidget.h
${CEE_CURRENT_LIST_DIR}RiuExportMultipleSnapshotsWidget.h
${CEE_CURRENT_LIST_DIR}RiuWellAllocationPlot.h
${CEE_CURRENT_LIST_DIR}RiuWellRftPlot.h
${CEE_CURRENT_LIST_DIR}RiuFlowCharacteristicsPlot.h
${CEE_CURRENT_LIST_DIR}RiuNightchartsWidget.h
${CEE_CURRENT_LIST_DIR}RiuMessagePanel.h
${CEE_CURRENT_LIST_DIR}RiuPlotObjectPicker.h
${CEE_CURRENT_LIST_DIR}RiuContextMenuLauncher.h
${CEE_CURRENT_LIST_DIR}RiuSummaryCurveDefinitionKeywords.h
${CEE_CURRENT_LIST_DIR}RiuSummaryCurveDefSelection.h
${CEE_CURRENT_LIST_DIR}RiuSummaryCurveDefSelectionDialog.h
${CEE_CURRENT_LIST_DIR}RiuSummaryCurveDefSelectionEditor.h
${CEE_CURRENT_LIST_DIR}RiuSummaryVectorDescriptionMap.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RiuCadNavigation.cpp
${CEE_CURRENT_LIST_DIR}RiuCursors.cpp
${CEE_CURRENT_LIST_DIR}RiuDragDrop.cpp
${CEE_CURRENT_LIST_DIR}RiuFemResultTextBuilder.cpp
${CEE_CURRENT_LIST_DIR}RiuGeoQuestNavigation.cpp
${CEE_CURRENT_LIST_DIR}RiuInterfaceToViewWindow.cpp
${CEE_CURRENT_LIST_DIR}RiuLineSegmentQwtPlotCurve.cpp
${CEE_CURRENT_LIST_DIR}RiuMainPlotWindow.cpp
${CEE_CURRENT_LIST_DIR}RiuMainWindow.cpp
${CEE_CURRENT_LIST_DIR}RiuMainWindowBase.cpp
${CEE_CURRENT_LIST_DIR}RiuMdiSubWindow.cpp
${CEE_CURRENT_LIST_DIR}RiuMultiCaseImportDialog.cpp
${CEE_CURRENT_LIST_DIR}RiuProcessMonitor.cpp
${CEE_CURRENT_LIST_DIR}RiuProjectPropertyView.cpp
${CEE_CURRENT_LIST_DIR}RiuPropertyViewTabWidget.cpp
${CEE_CURRENT_LIST_DIR}RiuQwtScalePicker.cpp
${CEE_CURRENT_LIST_DIR}RiuQwtCurvePointTracker.cpp
${CEE_CURRENT_LIST_DIR}RiuQwtPlotWheelZoomer.cpp
${CEE_CURRENT_LIST_DIR}RiuRecentFileActionProvider.cpp
${CEE_CURRENT_LIST_DIR}RiuResultInfoPanel.cpp
${CEE_CURRENT_LIST_DIR}RiuResultQwtPlot.cpp
${CEE_CURRENT_LIST_DIR}RiuResultTextBuilder.cpp
${CEE_CURRENT_LIST_DIR}RiuRmsNavigation.cpp
${CEE_CURRENT_LIST_DIR}RiuSelectionChangedHandler.cpp
${CEE_CURRENT_LIST_DIR}RiuSelectionManager.cpp
${CEE_CURRENT_LIST_DIR}RiuSimpleHistogramWidget.cpp
${CEE_CURRENT_LIST_DIR}RiuSummaryQwtPlot.cpp
${CEE_CURRENT_LIST_DIR}RiuTofAccumulatedPhaseFractionsPlot.cpp
${CEE_CURRENT_LIST_DIR}RiuToolTipMenu.cpp
${CEE_CURRENT_LIST_DIR}RiuTreeViewEventFilter.cpp
${CEE_CURRENT_LIST_DIR}RiuViewer.cpp
${CEE_CURRENT_LIST_DIR}RiuViewerCommands.cpp
${CEE_CURRENT_LIST_DIR}RiuWellLogPlot.cpp
${CEE_CURRENT_LIST_DIR}RiuWellLogTrack.cpp
${CEE_CURRENT_LIST_DIR}RiuGeoMechXfTensorResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RiuFemTimeHistoryResultAccessor.cpp
${CEE_CURRENT_LIST_DIR}RiuEditPerforationCollectionWidget.cpp
${CEE_CURRENT_LIST_DIR}RiuExportMultipleSnapshotsWidget.cpp
${CEE_CURRENT_LIST_DIR}RiuWellAllocationPlot.cpp
${CEE_CURRENT_LIST_DIR}RiuWellRftPlot.cpp
${CEE_CURRENT_LIST_DIR}RiuFlowCharacteristicsPlot.cpp
${CEE_CURRENT_LIST_DIR}RiuNightchartsWidget.cpp
${CEE_CURRENT_LIST_DIR}RiuMessagePanel.cpp
${CEE_CURRENT_LIST_DIR}RiuPlotObjectPicker.cpp
${CEE_CURRENT_LIST_DIR}RiuContextMenuLauncher.cpp
${CEE_CURRENT_LIST_DIR}RiuSummaryCurveDefSelection.cpp
${CEE_CURRENT_LIST_DIR}RiuSummaryCurveDefSelectionDialog.cpp
${CEE_CURRENT_LIST_DIR}RiuSummaryCurveDefSelectionEditor.cpp
${CEE_CURRENT_LIST_DIR}RiuSummaryVectorDescriptionMap.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

list(APPEND QT_MOC_HEADERS
${CEE_CURRENT_LIST_DIR}RiuMainWindowBase.h
${CEE_CURRENT_LIST_DIR}RiuMainWindow.h
${CEE_CURRENT_LIST_DIR}RiuMainPlotWindow.h
${CEE_CURRENT_LIST_DIR}RiuResultInfoPanel.h
${CEE_CURRENT_LIST_DIR}RiuViewer.h
${CEE_CURRENT_LIST_DIR}RiuProcessMonitor.h
${CEE_CURRENT_LIST_DIR}RiuMultiCaseImportDialog.h
${CEE_CURRENT_LIST_DIR}RiuViewerCommands.h
${CEE_CURRENT_LIST_DIR}RiuTreeViewEventFilter.h
${CEE_CURRENT_LIST_DIR}RiuWellLogPlot.h
${CEE_CURRENT_LIST_DIR}RiuWellLogTrack.h
${CEE_CURRENT_LIST_DIR}RiuRecentFileActionProvider.h
${CEE_CURRENT_LIST_DIR}RiuSummaryQwtPlot.h
${CEE_CURRENT_LIST_DIR}RiuTofAccumulatedPhaseFractionsPlot.h
${CEE_CURRENT_LIST_DIR}RiuQwtScalePicker.h
${CEE_CURRENT_LIST_DIR}RiuQwtPlotWheelZoomer.h
${CEE_CURRENT_LIST_DIR}RiuEditPerforationCollectionWidget.h
${CEE_CURRENT_LIST_DIR}RiuExportMultipleSnapshotsWidget.h
${CEE_CURRENT_LIST_DIR}RiuWellAllocationPlot.h
${CEE_CURRENT_LIST_DIR}RiuWellRftPlot.h
${CEE_CURRENT_LIST_DIR}RiuFlowCharacteristicsPlot.h
${CEE_CURRENT_LIST_DIR}RiuNightchartsWidget.h
${CEE_CURRENT_LIST_DIR}RiuMessagePanel.h
)

list(APPEND QT_UI_FILES
${CEE_CURRENT_LIST_DIR}RiuMultiCaseImportDialog.ui
)

source_group( "UserInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
