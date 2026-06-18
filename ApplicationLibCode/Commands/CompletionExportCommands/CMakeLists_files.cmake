set(SOURCE_GROUP_SOURCE_FILES
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
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathFractureTextReportFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathFractureReportItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForTemporaryLgrsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportMswTableData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MswExport/RicWellPathExportMswGeometryPath.cpp
    ${CMAKE_CURRENT_LIST_DIR}/MswExport/RicMswBranchBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicMswTableDataTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicScheduleDataGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicTransmissibilityCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPerforationCellFilterEvaluator.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
