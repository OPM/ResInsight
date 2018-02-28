
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionDataSettingsUi.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicFishbonesTransmissibilityCalculationFeatureImp.h
${CMAKE_CURRENT_LIST_DIR}/RigCompletionData.h
${CMAKE_CURRENT_LIST_DIR}/RigCompletionDataGridCell.h
${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesWellSegmentsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicCaseAndFileExportSettingsUi.h
)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES)
    list (APPEND SOURCE_GROUP_HEADER_FILES
        ${CMAKE_CURRENT_LIST_DIR}/RicExportFractureCompletionsImpl.h
        ${CMAKE_CURRENT_LIST_DIR}/RigEclipseToStimPlanCellTransmissibilityCalculator.h
        ${CMAKE_CURRENT_LIST_DIR}/RigTransmissibilityCondenser.h
        ${CMAKE_CURRENT_LIST_DIR}/RigFractureTransmissibilityEquations.h
        ${CMAKE_CURRENT_LIST_DIR}/RigWellPathStimplanIntersector.h
    )
endif()


set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RicExportCompletionDataSettingsUi.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicWellPathExportCompletionDataFeatureImpl.cpp
${CMAKE_CURRENT_LIST_DIR}/RicFishbonesTransmissibilityCalculationFeatureImp.cpp
${CMAKE_CURRENT_LIST_DIR}/RigCompletionData.cpp
${CMAKE_CURRENT_LIST_DIR}/RigCompletionDataGridCell.cpp
${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesWellSegmentsFeature.cpp
${CMAKE_CURRENT_LIST_DIR}/RicCaseAndFileExportSettingsUi.cpp
)

if (RESINSIGHT_ENABLE_PROTOTYPE_FEATURE_FRACTURES)
    list (APPEND SOURCE_GROUP_SOURCE_FILES
        ${CMAKE_CURRENT_LIST_DIR}/RicExportFractureCompletionsImpl.cpp
        ${CMAKE_CURRENT_LIST_DIR}/RigEclipseToStimPlanCellTransmissibilityCalculator.cpp
        ${CMAKE_CURRENT_LIST_DIR}/RigTransmissibilityCondenser.cpp
        ${CMAKE_CURRENT_LIST_DIR}/RigFractureTransmissibilityEquations.cpp
        ${CMAKE_CURRENT_LIST_DIR}/RigWellPathStimplanIntersector.cpp
    )
endif()


list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\CompletionExport" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
