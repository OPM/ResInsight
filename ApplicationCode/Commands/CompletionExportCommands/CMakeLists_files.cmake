
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RicExportCompletionDataSettingsUi.h
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.h
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeatureImpl.h
${CEE_CURRENT_LIST_DIR}RicFishbonesTransmissibilityCalculationFeatureImp.h
${CEE_CURRENT_LIST_DIR}RigCompletionData.h
${CEE_CURRENT_LIST_DIR}RigCompletionDataGridCell.h
${CEE_CURRENT_LIST_DIR}RicExportFishbonesWellSegmentsFeature.h
${CEE_CURRENT_LIST_DIR}RicCaseAndFileExportSettingsUi.h
${CEE_CURRENT_LIST_DIR}RicExportFractureCompletionsImpl.h
${CEE_CURRENT_LIST_DIR}RigEclipseToStimPlanCellTransmissibilityCalculator.h
${CEE_CURRENT_LIST_DIR}RigTransmissibilityCondenser.h
${CEE_CURRENT_LIST_DIR}RigFractureTransmissibilityEquations.h
${CEE_CURRENT_LIST_DIR}RigWellPathStimplanIntersector.h
)


set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RicExportCompletionDataSettingsUi.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeature.cpp
${CEE_CURRENT_LIST_DIR}RicWellPathExportCompletionDataFeatureImpl.cpp
${CEE_CURRENT_LIST_DIR}RicFishbonesTransmissibilityCalculationFeatureImp.cpp
${CEE_CURRENT_LIST_DIR}RigCompletionData.cpp
${CEE_CURRENT_LIST_DIR}RigCompletionDataGridCell.cpp
${CEE_CURRENT_LIST_DIR}RicExportFishbonesWellSegmentsFeature.cpp
${CEE_CURRENT_LIST_DIR}RicCaseAndFileExportSettingsUi.cpp
${CEE_CURRENT_LIST_DIR}RicExportFractureCompletionsImpl.cpp
${CEE_CURRENT_LIST_DIR}RigEclipseToStimPlanCellTransmissibilityCalculator.cpp
${CEE_CURRENT_LIST_DIR}RigTransmissibilityCondenser.cpp
${CEE_CURRENT_LIST_DIR}RigFractureTransmissibilityEquations.cpp
${CEE_CURRENT_LIST_DIR}RigWellPathStimplanIntersector.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\CompletionExport" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
