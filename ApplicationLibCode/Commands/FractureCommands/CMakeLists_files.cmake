set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicPasteEllipseFractureFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicPasteStimPlanFractureFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToFieldFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicConvertAllFractureTemplatesToMetricFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateDuplicateTemplateInOtherUnitSystemFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicFractureNameGenerator.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewEllipseFractureTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanFractureTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewThermalFractureTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureAtPosFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathFractureFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanModelFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewWellPathStimPlanModelAtPosFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanModelTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewElasticPropertyScalingFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultipleFracturesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultipleFracturesOptionItemUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateMultipleFracturesUi.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewOptionItemFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteOptionItemFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicPlaceThermalFractureUsingTemplateDataFeature.h
)

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

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
