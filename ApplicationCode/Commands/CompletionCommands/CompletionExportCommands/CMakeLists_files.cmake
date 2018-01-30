
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicExportCompletionDataSettingsUi.h
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.h
${CEE_CURRENT_LIST_DIR}RicFishbonesTransmissibilityCalculationFeatureImp.h
)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES)
    list (APPEND SOURCE_GROUP_HEADER_FILES
        ${CEE_CURRENT_LIST_DIR}RicExportFractureCompletionsImpl.h
    )
endif()


set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicExportCompletionDataSettingsUi.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.cpp
${CEE_CURRENT_LIST_DIR}RicFishbonesTransmissibilityCalculationFeatureImp.cpp
)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES)
    list (APPEND SOURCE_GROUP_SOURCE_FILES
        ${CEE_CURRENT_LIST_DIR}RicExportFractureCompletionsImpl.cpp
    )
endif()


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\CompletionExport" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
