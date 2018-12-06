
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionDataSettingsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportMswCompletionsImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionsFileTools.h
${CMAKE_CURRENT_LIST_DIR}/RicFishbonesTransmissibilityCalculationFeatureImp.h
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsWellSegmentsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCaseAndFileExportSettingsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFractureCompletionsImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForVisibleWellPathsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForVisibleSimWellsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicMswCompletions.h
${CMAKE_CURRENT_LIST_DIR}/RicMswExportInfo.h
${CMAKE_CURRENT_LIST_DIR}/RicMswSegment.h
${CMAKE_CURRENT_LIST_DIR}/RicMswSubSegment.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFractureTextReportFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFractureReportItem.h
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForTemporaryLgrsFeature.h
)


set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionDataSettingsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportMswCompletionsImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionsFileTools.cpp
${CMAKE_CURRENT_LIST_DIR}/RicFishbonesTransmissibilityCalculationFeatureImp.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsWellSegmentsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCaseAndFileExportSettingsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFractureCompletionsImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForVisibleWellPathsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForVisibleSimWellsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicMswCompletions.cpp
${CMAKE_CURRENT_LIST_DIR}/RicMswExportInfo.cpp
${CMAKE_CURRENT_LIST_DIR}/RicMswSegment.cpp
${CMAKE_CURRENT_LIST_DIR}/RicMswSubSegment.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFractureTextReportFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathFractureReportItem.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForTemporaryLgrsFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\CompletionExport" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
