
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RimEclipseCaseCollection.h
${CEE_CURRENT_LIST_DIR}RimCaseCollection.h
${CEE_CURRENT_LIST_DIR}RimCellFilter.h
${CEE_CURRENT_LIST_DIR}RimEclipsePropertyFilter.h
${CEE_CURRENT_LIST_DIR}RimPropertyFilterCollection.h
${CEE_CURRENT_LIST_DIR}RimEclipsePropertyFilterCollection.h
${CEE_CURRENT_LIST_DIR}RimCellRangeFilter.h
${CEE_CURRENT_LIST_DIR}RimCellRangeFilterCollection.h
${CEE_CURRENT_LIST_DIR}RimLegendConfig.h
${CEE_CURRENT_LIST_DIR}RimOilField.h
${CEE_CURRENT_LIST_DIR}RimProject.h
${CEE_CURRENT_LIST_DIR}RimEclipseCase.h
${CEE_CURRENT_LIST_DIR}RimIdenticalGridCaseGroup.h
${CEE_CURRENT_LIST_DIR}RimEclipseInputProperty.h
${CEE_CURRENT_LIST_DIR}RimEclipseInputPropertyCollection.h
${CEE_CURRENT_LIST_DIR}RimEclipseInputCase.h
${CEE_CURRENT_LIST_DIR}RimEclipseResultCase.h
${CEE_CURRENT_LIST_DIR}RimEclipseView.h
${CEE_CURRENT_LIST_DIR}RimEclipseResultDefinition.h
${CEE_CURRENT_LIST_DIR}RimEclipseCellColors.h
${CEE_CURRENT_LIST_DIR}RimCellEdgeColors.h
${CEE_CURRENT_LIST_DIR}RimSimWellInView.h
${CEE_CURRENT_LIST_DIR}RimSimWellInViewCollection.h
${CEE_CURRENT_LIST_DIR}RimWellPath.h
${CEE_CURRENT_LIST_DIR}RimWellPathCollection.h
${CEE_CURRENT_LIST_DIR}RimScriptCollection.h
${CEE_CURRENT_LIST_DIR}RimEclipseStatisticsCase.h
${CEE_CURRENT_LIST_DIR}RimEclipseStatisticsCaseCollection.h
${CEE_CURRENT_LIST_DIR}RimCalcScript.h
${CEE_CURRENT_LIST_DIR}RimExportInputPropertySettings.h
${CEE_CURRENT_LIST_DIR}RimBinaryExportSettings.h
${CEE_CURRENT_LIST_DIR}Rim3dOverlayInfoConfig.h
${CEE_CURRENT_LIST_DIR}RimReservoirCellResultsStorage.h
${CEE_CURRENT_LIST_DIR}RimEclipseStatisticsCaseEvaluator.h
${CEE_CURRENT_LIST_DIR}RimMimeData.h
${CEE_CURRENT_LIST_DIR}RimCommandObject.h
${CEE_CURRENT_LIST_DIR}RimTools.h
${CEE_CURRENT_LIST_DIR}RimFaultInView.h
${CEE_CURRENT_LIST_DIR}RimFaultCollection.h
${CEE_CURRENT_LIST_DIR}RimFormationNames.h
${CEE_CURRENT_LIST_DIR}RimFormationNamesCollection.h
${CEE_CURRENT_LIST_DIR}RimMockModelSettings.h
${CEE_CURRENT_LIST_DIR}RimTernaryLegendConfig.h
${CEE_CURRENT_LIST_DIR}RimEclipseFaultColors.h
${CEE_CURRENT_LIST_DIR}RimNoCommonAreaNNC.h
${CEE_CURRENT_LIST_DIR}RimNoCommonAreaNncCollection.h
${CEE_CURRENT_LIST_DIR}RimGeoMechModels.h
${CEE_CURRENT_LIST_DIR}RimGeoMechCase.h
${CEE_CURRENT_LIST_DIR}RimGeoMechView.h
${CEE_CURRENT_LIST_DIR}RimGeoMechPropertyFilter.h
${CEE_CURRENT_LIST_DIR}RimGeoMechPropertyFilterCollection.h
${CEE_CURRENT_LIST_DIR}RimGeoMechResultDefinition.h
${CEE_CURRENT_LIST_DIR}RimGeoMechCellColors.h
${CEE_CURRENT_LIST_DIR}RimViewWindow.h
${CEE_CURRENT_LIST_DIR}RimView.h
${CEE_CURRENT_LIST_DIR}RimViewManipulator.h
${CEE_CURRENT_LIST_DIR}RimCase.h
${CEE_CURRENT_LIST_DIR}RimViewController.h
${CEE_CURRENT_LIST_DIR}RimMainPlotCollection.h
${CEE_CURRENT_LIST_DIR}RimWellLogPlotCollection.h
${CEE_CURRENT_LIST_DIR}RimRftPlotCollection.h
${CEE_CURRENT_LIST_DIR}RimWellLogPlot.h
${CEE_CURRENT_LIST_DIR}RimWellLogTrack.h
${CEE_CURRENT_LIST_DIR}RimWellLogCurve.h
${CEE_CURRENT_LIST_DIR}RimViewLinker.h
${CEE_CURRENT_LIST_DIR}RimViewLinkerCollection.h
${CEE_CURRENT_LIST_DIR}RimWellLogExtractionCurve.h
${CEE_CURRENT_LIST_DIR}RimWellLogFile.h
${CEE_CURRENT_LIST_DIR}RimWellLogFileChannel.h
${CEE_CURRENT_LIST_DIR}RimWellLogFileCurve.h
${CEE_CURRENT_LIST_DIR}RimWellLogRftCurve.h
${CEE_CURRENT_LIST_DIR}RimIntersection.h
${CEE_CURRENT_LIST_DIR}RimIntersectionCollection.h
${CEE_CURRENT_LIST_DIR}RimContextCommandBuilder.h
${CEE_CURRENT_LIST_DIR}RimGridCollection.h
${CEE_CURRENT_LIST_DIR}RimPlotCurve.h
${CEE_CURRENT_LIST_DIR}RimIntersectionBox.h
${CEE_CURRENT_LIST_DIR}RimMultiSnapshotDefinition.h
${CEE_CURRENT_LIST_DIR}RimMdiWindowController.h
${CEE_CURRENT_LIST_DIR}RimPropertyFilter.h
${CEE_CURRENT_LIST_DIR}RimNamedObject.h
${CEE_CURRENT_LIST_DIR}RimCheckableNamedObject.h
${CEE_CURRENT_LIST_DIR}RimGridTimeHistoryCurve.h
${CEE_CURRENT_LIST_DIR}RimGeometrySelectionItem.h
${CEE_CURRENT_LIST_DIR}RimEclipseGeometrySelectionItem.h
${CEE_CURRENT_LIST_DIR}RimDialogData.h
${CEE_CURRENT_LIST_DIR}RimTimeStepFilter.h
${CEE_CURRENT_LIST_DIR}RimCalculation.h
${CEE_CURRENT_LIST_DIR}RimCalculationCollection.h
${CEE_CURRENT_LIST_DIR}RimCalculationVariable.h
)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES) 
    list (APPEND SOURCE_GROUP_HEADER_FILES 
        ${CEE_CURRENT_LIST_DIR}RimStimPlanLegendConfig.h
        ${CEE_CURRENT_LIST_DIR}RimStimPlanColors.h
    ) 
endif() 

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RimEclipseCaseCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCaseCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCellFilter.cpp
${CEE_CURRENT_LIST_DIR}RimEclipsePropertyFilter.cpp
${CEE_CURRENT_LIST_DIR}RimPropertyFilterCollection.cpp
${CEE_CURRENT_LIST_DIR}RimEclipsePropertyFilterCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCellRangeFilter.cpp
${CEE_CURRENT_LIST_DIR}RimCellRangeFilterCollection.cpp
${CEE_CURRENT_LIST_DIR}RimLegendConfig.cpp
${CEE_CURRENT_LIST_DIR}RimOilField.cpp
${CEE_CURRENT_LIST_DIR}RimProject.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseCase.cpp
${CEE_CURRENT_LIST_DIR}RimIdenticalGridCaseGroup.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseInputProperty.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseInputPropertyCollection.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseInputCase.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseResultCase.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseView.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseResultDefinition.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseCellColors.cpp
${CEE_CURRENT_LIST_DIR}RimCellEdgeColors.cpp
${CEE_CURRENT_LIST_DIR}RimSimWellInView.cpp
${CEE_CURRENT_LIST_DIR}RimSimWellInViewCollection.cpp
${CEE_CURRENT_LIST_DIR}RimWellPath.cpp
${CEE_CURRENT_LIST_DIR}RimWellPathCollection.cpp
${CEE_CURRENT_LIST_DIR}RimScriptCollection.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseStatisticsCase.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseStatisticsCaseCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCalcScript.cpp
${CEE_CURRENT_LIST_DIR}RimExportInputPropertySettings.cpp
${CEE_CURRENT_LIST_DIR}RimBinaryExportSettings.cpp
${CEE_CURRENT_LIST_DIR}Rim3dOverlayInfoConfig.cpp
${CEE_CURRENT_LIST_DIR}RimReservoirCellResultsStorage.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseStatisticsCaseEvaluator.cpp
${CEE_CURRENT_LIST_DIR}RimMimeData.cpp
${CEE_CURRENT_LIST_DIR}RimCommandObject.cpp
${CEE_CURRENT_LIST_DIR}RimTools.cpp
${CEE_CURRENT_LIST_DIR}RimFaultInView.cpp
${CEE_CURRENT_LIST_DIR}RimFaultCollection.cpp
${CEE_CURRENT_LIST_DIR}RimFormationNames.cpp
${CEE_CURRENT_LIST_DIR}RimFormationNamesCollection.cpp
${CEE_CURRENT_LIST_DIR}RimMockModelSettings.cpp
${CEE_CURRENT_LIST_DIR}RimTernaryLegendConfig.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseFaultColors.cpp
${CEE_CURRENT_LIST_DIR}RimNoCommonAreaNNC.cpp
${CEE_CURRENT_LIST_DIR}RimNoCommonAreaNncCollection.cpp
${CEE_CURRENT_LIST_DIR}RimGeoMechModels.cpp
${CEE_CURRENT_LIST_DIR}RimGeoMechCase.cpp
${CEE_CURRENT_LIST_DIR}RimGeoMechView.cpp
${CEE_CURRENT_LIST_DIR}RimGeoMechPropertyFilter.cpp
${CEE_CURRENT_LIST_DIR}RimGeoMechPropertyFilterCollection.cpp
${CEE_CURRENT_LIST_DIR}RimGeoMechResultDefinition.cpp
${CEE_CURRENT_LIST_DIR}RimGeoMechCellColors.cpp
${CEE_CURRENT_LIST_DIR}RimViewWindow.cpp
${CEE_CURRENT_LIST_DIR}RimView.cpp
${CEE_CURRENT_LIST_DIR}RimViewManipulator.cpp
${CEE_CURRENT_LIST_DIR}RimCase.cpp
${CEE_CURRENT_LIST_DIR}RimViewController.cpp
${CEE_CURRENT_LIST_DIR}RimMainPlotCollection.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogPlotCollection.cpp
${CEE_CURRENT_LIST_DIR}RimRftPlotCollection.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogPlot.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogTrack.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogCurve.cpp
${CEE_CURRENT_LIST_DIR}RimViewLinker.cpp
${CEE_CURRENT_LIST_DIR}RimViewLinkerCollection.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogExtractionCurve.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogFile.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogFileChannel.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogFileCurve.cpp
${CEE_CURRENT_LIST_DIR}RimWellLogRftCurve.cpp
${CEE_CURRENT_LIST_DIR}RimIntersection.cpp
${CEE_CURRENT_LIST_DIR}RimIntersectionCollection.cpp
${CEE_CURRENT_LIST_DIR}RimContextCommandBuilder.cpp
${CEE_CURRENT_LIST_DIR}RimGridCollection.cpp
${CEE_CURRENT_LIST_DIR}RimPlotCurve.cpp
${CEE_CURRENT_LIST_DIR}RimIntersectionBox.cpp
${CEE_CURRENT_LIST_DIR}RimMultiSnapshotDefinition.cpp
${CEE_CURRENT_LIST_DIR}RimMdiWindowController.cpp
${CEE_CURRENT_LIST_DIR}RimPropertyFilter.cpp
${CEE_CURRENT_LIST_DIR}RimNamedObject.cpp
${CEE_CURRENT_LIST_DIR}RimCheckableNamedObject.cpp
${CEE_CURRENT_LIST_DIR}RimGridTimeHistoryCurve.cpp
${CEE_CURRENT_LIST_DIR}RimGeometrySelectionItem.cpp
${CEE_CURRENT_LIST_DIR}RimEclipseGeometrySelectionItem.cpp
${CEE_CURRENT_LIST_DIR}RimDialogData.cpp
${CEE_CURRENT_LIST_DIR}RimTimeStepFilter.cpp
${CEE_CURRENT_LIST_DIR}RimCalculation.cpp
${CEE_CURRENT_LIST_DIR}RimCalculationCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCalculationVariable.cpp
)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES)
    list (APPEND SOURCE_GROUP_SOURCE_FILES
        ${CEE_CURRENT_LIST_DIR}RimStimPlanLegendConfig.cpp
        ${CEE_CURRENT_LIST_DIR}RimStimPlanColors.cpp
    )
endif()

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)


source_group( "ProjectDataModel" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
