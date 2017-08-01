
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicCaseAndFileExportSettingsUi.h
${CEE_CURRENT_LIST_DIR}RicEditPerforationCollectionFeature.h
${CEE_CURRENT_LIST_DIR}RicExportCompletionDataSettingsUi.h
${CEE_CURRENT_LIST_DIR}RicExportFishbonesLateralsFeature.h
${CEE_CURRENT_LIST_DIR}RicExportFishbonesWellSegmentsFeature.h
${CEE_CURRENT_LIST_DIR}RicExportFractureCompletionsImpl.h
${CEE_CURRENT_LIST_DIR}RicNewFishbonesSubsAtMeasuredDepthFeature.h
${CEE_CURRENT_LIST_DIR}RicNewFishbonesSubsFeature.h
${CEE_CURRENT_LIST_DIR}RicNewPerforationIntervalFeature.h
${CEE_CURRENT_LIST_DIR}RicNewPerforationIntervalAtMeasuredDepthFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathImportCompletionsFileFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathImportPerforationIntervalsFeature.h
${CEE_CURRENT_LIST_DIR}RicFishbonesTransmissibilityCalculationFeatureImp.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicCaseAndFileExportSettingsUi.cpp
${CEE_CURRENT_LIST_DIR}RicEditPerforationCollectionFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportCompletionDataSettingsUi.cpp
${CEE_CURRENT_LIST_DIR}RicExportFishbonesLateralsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportFishbonesWellSegmentsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicExportFractureCompletionsImpl.cpp
${CEE_CURRENT_LIST_DIR}RicNewFishbonesSubsAtMeasuredDepthFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewFishbonesSubsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewPerforationIntervalFeature.cpp
${CEE_CURRENT_LIST_DIR}RicNewPerforationIntervalAtMeasuredDepthFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathImportCompletionsFileFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathImportPerforationIntervalsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicFishbonesTransmissibilityCalculationFeatureImp.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\Completion" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
