
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RiuCadNavigation.h
${CMAKE_CURRENT_LIST_DIR}/RiuCursors.h
${CMAKE_CURRENT_LIST_DIR}/RiuDragDrop.h
${CMAKE_CURRENT_LIST_DIR}/RiuFemResultTextBuilder.h
${CMAKE_CURRENT_LIST_DIR}/RiuGeoQuestNavigation.h
${CMAKE_CURRENT_LIST_DIR}/RiuInterfaceToViewWindow.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtSymbol.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotCurve.h
${CMAKE_CURRENT_LIST_DIR}/RiuRimQwtPlotCurve.h
${CMAKE_CURRENT_LIST_DIR}/RiuPlotMainWindow.h
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindow.h
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindowBase.h
${CMAKE_CURRENT_LIST_DIR}/RiuMdiArea.h
${CMAKE_CURRENT_LIST_DIR}/RiuMdiSubWindow.h
${CMAKE_CURRENT_LIST_DIR}/RiuMultiCaseImportDialog.h
${CMAKE_CURRENT_LIST_DIR}/RiuProcessMonitor.h
${CMAKE_CURRENT_LIST_DIR}/RiuProjectPropertyView.h
${CMAKE_CURRENT_LIST_DIR}/RiuPropertyViewTabWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuPvtPlotPanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuPvtPlotUpdater.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtLinearScaleEngine.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtScalePicker.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtCurvePointTracker.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotWheelZoomer.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotZoomer.h
${CMAKE_CURRENT_LIST_DIR}/RiuWidgetDragger.h
${CMAKE_CURRENT_LIST_DIR}/RiuRecentFileActionProvider.h
${CMAKE_CURRENT_LIST_DIR}/RiuRelativePermeabilityPlotPanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuRelativePermeabilityPlotUpdater.h
${CMAKE_CURRENT_LIST_DIR}/RiuResultInfoPanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuResultQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuResultTextBuilder.h
${CMAKE_CURRENT_LIST_DIR}/RiuRmsNavigation.h
${CMAKE_CURRENT_LIST_DIR}/RiuSelectionChangedHandler.h
${CMAKE_CURRENT_LIST_DIR}/Riu3dSelectionManager.h
${CMAKE_CURRENT_LIST_DIR}/RiuSimpleHistogramWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuDockedQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuGridCrossQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuTextDialog.h
${CMAKE_CURRENT_LIST_DIR}/RiuTimeStepChangedHandler.h
${CMAKE_CURRENT_LIST_DIR}/RiuTofAccumulatedPhaseFractionsPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuToolTipMenu.h
${CMAKE_CURRENT_LIST_DIR}/RiuTreeViewEventFilter.h
${CMAKE_CURRENT_LIST_DIR}/RiuViewer.h
${CMAKE_CURRENT_LIST_DIR}/RiuViewerToViewInterface.h
${CMAKE_CURRENT_LIST_DIR}/RiuViewerCommands.h
${CMAKE_CURRENT_LIST_DIR}/RiuCellAndNncPickEventHandler.h
${CMAKE_CURRENT_LIST_DIR}/RiuPickItemInfo.h
${CMAKE_CURRENT_LIST_DIR}/RiuWellLogPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuWellLogTrack.h
${CMAKE_CURRENT_LIST_DIR}/RiuMultiPlotPage.h
${CMAKE_CURRENT_LIST_DIR}/RiuMultiPlotBook.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotLegend.h
${CMAKE_CURRENT_LIST_DIR}/RiuPlotAnnotationTool.h
${CMAKE_CURRENT_LIST_DIR}/RiuGeoMechXfTensorResultAccessor.h
${CMAKE_CURRENT_LIST_DIR}/RiuFemTimeHistoryResultAccessor.h
${CMAKE_CURRENT_LIST_DIR}/RiuEditPerforationCollectionWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuAdvancedSnapshotExportWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuWellAllocationPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuFlowCharacteristicsPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuNightchartsWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuMessagePanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuPlotObjectPicker.h
${CMAKE_CURRENT_LIST_DIR}/RiuContextMenuLauncher.h
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryCurveDefinitionKeywords.h
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryVectorSelectionUi.h
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryVectorSelectionDialog.h
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryVectorSelectionWidgetCreator.h
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryQuantityNameInfoProvider.h
${CMAKE_CURRENT_LIST_DIR}/RiuExpressionContextMenuManager.h
${CMAKE_CURRENT_LIST_DIR}/RiuCalculationsContextMenuManager.h
${CMAKE_CURRENT_LIST_DIR}/RiuGridStatisticsHistogramWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuTools.h
${CMAKE_CURRENT_LIST_DIR}/RiuMohrsCirclePlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuPlotMainWindowTools.h
${CMAKE_CURRENT_LIST_DIR}/Riu3DMainWindowTools.h
${CMAKE_CURRENT_LIST_DIR}/RiuDockWidgetTools.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotItemGroup.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotTools.h
${CMAKE_CURRENT_LIST_DIR}/RiuWellPathComponentPlotItem.h
${CMAKE_CURRENT_LIST_DIR}/RiuMeasurementViewEventFilter.h
${CMAKE_CURRENT_LIST_DIR}/RiuDraggableOverlayFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuMdiMaximizeWindowGuard.h
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindowTools.h
${CMAKE_CURRENT_LIST_DIR}/RiuComparisonViewMover.h
${CMAKE_CURRENT_LIST_DIR}/RiuAbstractOverlayContentFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuAbstractLegendFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuCategoryLegendFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuScalarMapperLegendFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuFileDialogTools.h
${CMAKE_CURRENT_LIST_DIR}/RiuGuiTheme.h
${CMAKE_CURRENT_LIST_DIR}/RiuQssSyntaxHighlighter.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RiuCadNavigation.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuCursors.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuDragDrop.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuFemResultTextBuilder.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuGeoQuestNavigation.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuInterfaceToViewWindow.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtSymbol.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuRimQwtPlotCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPlotMainWindow.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindow.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindowBase.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMdiArea.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMdiSubWindow.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMultiCaseImportDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuProcessMonitor.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuProjectPropertyView.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPropertyViewTabWidget.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPvtPlotPanel.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPvtPlotUpdater.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtLinearScaleEngine.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtScalePicker.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtCurvePointTracker.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotWheelZoomer.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuWidgetDragger.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuRecentFileActionProvider.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuRelativePermeabilityPlotPanel.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuRelativePermeabilityPlotUpdater.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuResultInfoPanel.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuResultQwtPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuResultTextBuilder.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuRmsNavigation.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuSelectionChangedHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/Riu3dSelectionManager.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuSimpleHistogramWidget.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuDockedQwtPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuGridCrossQwtPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryQwtPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuTextDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuTimeStepChangedHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuTofAccumulatedPhaseFractionsPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuToolTipMenu.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuTreeViewEventFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuViewer.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuViewerCommands.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuCellAndNncPickEventHandler.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPickItemInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuWellLogTrack.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuWellLogPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMultiPlotPage.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMultiPlotBook.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotWidget.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotLegend.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPlotAnnotationTool.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuGeoMechXfTensorResultAccessor.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuFemTimeHistoryResultAccessor.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuEditPerforationCollectionWidget.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuAdvancedSnapshotExportWidget.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuWellAllocationPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuFlowCharacteristicsPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuNightchartsWidget.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMessagePanel.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPlotObjectPicker.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuContextMenuLauncher.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryVectorSelectionUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryVectorSelectionDialog.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryVectorSelectionWidgetCreator.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryQuantityNameInfoProvider.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuExpressionContextMenuManager.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuCalculationsContextMenuManager.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuGridStatisticsHistogramWidget.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMohrsCirclePlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuPlotMainWindowTools.cpp
${CMAKE_CURRENT_LIST_DIR}/Riu3DMainWindowTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuDockWidgetTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotItemGroup.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuWellPathComponentPlotItem.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuDraggableOverlayFrame.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMdiMaximizeWindowGuard.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindowTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuComparisonViewMover.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuAbstractOverlayContentFrame.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuAbstractLegendFrame.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuCategoryLegendFrame.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuScalarMapperLegendFrame.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuFileDialogTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuGuiTheme.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuQssSyntaxHighlighter.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuTextEditWithCompletion.cpp
${CMAKE_CURRENT_LIST_DIR}/RiuTextContentFrame.cpp
)


if(Qt5Charts_FOUND)
  list(APPEND CODE_HEADER_FILES
       ${CMAKE_CURRENT_LIST_DIR}/RiuQtChartView.h)

  list(APPEND CODE_SOURCE_FILES
       ${CMAKE_CURRENT_LIST_DIR}/RiuQtChartView.cpp)

#  list(APPEND QT_MOC_HEADERS
#       ${CMAKE_CURRENT_LIST_DIR}/RiuQtChartView.h)
endif()



list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)


list(APPEND QT_MOC_HEADERS
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindowBase.h
${CMAKE_CURRENT_LIST_DIR}/RiuMainWindow.h
${CMAKE_CURRENT_LIST_DIR}/RiuPlotMainWindow.h
${CMAKE_CURRENT_LIST_DIR}/RiuMdiArea.h
${CMAKE_CURRENT_LIST_DIR}/RiuMdiSubWindow.h
${CMAKE_CURRENT_LIST_DIR}/RiuPvtPlotPanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuRelativePermeabilityPlotPanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuResultInfoPanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuResultQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuTextDialog.h
${CMAKE_CURRENT_LIST_DIR}/RiuViewer.h
${CMAKE_CURRENT_LIST_DIR}/RiuProcessMonitor.h
${CMAKE_CURRENT_LIST_DIR}/RiuMultiCaseImportDialog.h
${CMAKE_CURRENT_LIST_DIR}/RiuViewerCommands.h
${CMAKE_CURRENT_LIST_DIR}/RiuTreeViewEventFilter.h
${CMAKE_CURRENT_LIST_DIR}/RiuWellLogPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuWellLogTrack.h
${CMAKE_CURRENT_LIST_DIR}/RiuMultiPlotPage.h
${CMAKE_CURRENT_LIST_DIR}/RiuMultiPlotBook.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotLegend.h
${CMAKE_CURRENT_LIST_DIR}/RiuRecentFileActionProvider.h
${CMAKE_CURRENT_LIST_DIR}/RiuDockedQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuGridCrossQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuSummaryQwtPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuTofAccumulatedPhaseFractionsPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtScalePicker.h
${CMAKE_CURRENT_LIST_DIR}/RiuQwtPlotWheelZoomer.h
${CMAKE_CURRENT_LIST_DIR}/RiuWidgetDragger.h
${CMAKE_CURRENT_LIST_DIR}/RiuEditPerforationCollectionWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuAdvancedSnapshotExportWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuWellAllocationPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuFlowCharacteristicsPlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuNightchartsWidget.h
${CMAKE_CURRENT_LIST_DIR}/RiuMessagePanel.h
${CMAKE_CURRENT_LIST_DIR}/RiuExpressionContextMenuManager.h
${CMAKE_CURRENT_LIST_DIR}/RiuCalculationsContextMenuManager.h
${CMAKE_CURRENT_LIST_DIR}/RiuMohrsCirclePlot.h
${CMAKE_CURRENT_LIST_DIR}/RiuDraggableOverlayFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuAbstractOverlayContentFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuAbstractLegendFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuCategoryLegendFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuScalarMapperLegendFrame.h
${CMAKE_CURRENT_LIST_DIR}/RiuTextEditWithCompletion.h
${CMAKE_CURRENT_LIST_DIR}/RiuTextContentFrame.h
)

list(APPEND QT_UI_FILES
${CMAKE_CURRENT_LIST_DIR}/RiuMultiCaseImportDialog.ui
)





source_group( "UserInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
