set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicPasteEllipseFractureFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPasteStimPlanFractureFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToFieldFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToMetricFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDuplicateTemplateInOtherUnitSystemFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicFractureNameGenerator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewEllipseFractureTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanFractureTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewThermalFractureTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureAtPosFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanModelFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathStimPlanModelAtPosFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanModelTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewElasticPropertyScalingFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultipleFracturesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultipleFracturesOptionItemUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultipleFracturesUi.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewOptionItemFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteOptionItemFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPlaceThermalFractureUsingTemplateDataFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
