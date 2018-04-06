
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionDataSettingsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicFishbonesTransmissibilityCalculationFeatureImp.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesWellSegmentsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCaseAndFileExportSettingsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFractureCompletionsImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForVisibleWellPathsFeature.h
)


set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionDataSettingsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicFishbonesTransmissibilityCalculationFeatureImp.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesWellSegmentsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCaseAndFileExportSettingsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFractureCompletionsImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionsForVisibleWellPathsFeature.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\CompletionExport" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
