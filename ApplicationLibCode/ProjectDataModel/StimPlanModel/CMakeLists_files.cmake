set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelTemplate.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModel.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelTemplateCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelElasticPropertyCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelLayerCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelStressCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelWellLogCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPressureCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimElasticProperties.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScaling.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScalingCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaciesProperties.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimNonNetLayers.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFaciesInitialPressureConfig.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPressureTableItem.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimPressureTable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimExtractionConfiguration.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
