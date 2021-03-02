
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicfCloseProject.h
${CMAKE_CURRENT_LIST_DIR}/RicfCommandFileExecutor.h
${CMAKE_CURRENT_LIST_DIR}/RicfComputeCaseGroupStatistics.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportMsw.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportMultiCaseSnapshots.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportProperty.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportSimWellFractureCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportSnapshots.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPathCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RicfLoadCase.h
${CMAKE_CURRENT_LIST_DIR}/RicfOpenProject.h
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceCase.h
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceSourceCases.h
${CMAKE_CURRENT_LIST_DIR}/RicfRunOctaveScript.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetExportFolder.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetMainWindowSize.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetPlotWindowSize.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetStartDir.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetTimeStep.h
${CMAKE_CURRENT_LIST_DIR}/RicfScaleFractureTemplate.h
${CMAKE_CURRENT_LIST_DIR}/RicfSetFractureContainment.h
${CMAKE_CURRENT_LIST_DIR}/RicfCreateMultipleFractures.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPaths.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportVisibleCells.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportPropertyInViews.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportLgrForCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RicfCreateLgrForCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RicfApplicationTools.h
${CMAKE_CURRENT_LIST_DIR}/RicfCreateSaturationPressurePlots.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportFlowCharacteristics.h
${CMAKE_CURRENT_LIST_DIR}/RicfCreateGridCaseGroup.h
${CMAKE_CURRENT_LIST_DIR}/RicfCreateStatisticsCase.h
${CMAKE_CURRENT_LIST_DIR}/RicfCreateView.h
${CMAKE_CURRENT_LIST_DIR}/RicfCloneView.h
${CMAKE_CURRENT_LIST_DIR}/RicfNewWellBoreStabilityPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicfImportWellLogFiles.h
${CMAKE_CURRENT_LIST_DIR}/RicfImportFormationNames.h
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellLogPlotData.h
${CMAKE_CURRENT_LIST_DIR}/RicfEnableDataSourceAsComment.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicfCloseProject.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCommandFileExecutor.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfComputeCaseGroupStatistics.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportMsw.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportMultiCaseSnapshots.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportProperty.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportSimWellFractureCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportSnapshots.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPathCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfLoadCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfOpenProject.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfReplaceSourceCases.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfRunOctaveScript.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetExportFolder.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetMainWindowSize.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetPlotWindowSize.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetStartDir.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetTimeStep.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfScaleFractureTemplate.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfSetFractureContainment.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateMultipleFractures.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellPaths.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportVisibleCells.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportPropertyInViews.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportLgrForCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateLgrForCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfApplicationTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateSaturationPressurePlots.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportFlowCharacteristics.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateGridCaseGroup.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateStatisticsCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateView.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCloneView.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfCreateWellBoreStabilityPlotFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfImportWellLogFiles.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfImportFormationNames.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfExportWellLogPlotData.cpp
${CMAKE_CURRENT_LIST_DIR}/RicfEnableDataSourceAsComment.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFileInterface" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
