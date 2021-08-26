set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelTemplate.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModel.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelTemplateCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelElasticPropertyCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelLayerCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPropertyCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPropertyCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelStressCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelWellLogCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimStimPlanModelPressureCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimElasticProperties.h
    ${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScaling.h
    ${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScalingCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaciesProperties.h
    ${CMAKE_CURRENT_LIST_DIR}/RimNonNetLayers.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFaciesInitialPressureConfig.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPressureTableItem.h
    ${CMAKE_CURRENT_LIST_DIR}/RimPressureTable.h
    ${CMAKE_CURRENT_LIST_DIR}/RimExtractionConfiguration.h)

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
    ${CMAKE_CURRENT_LIST_DIR}/RimExtractionConfiguration.cpp)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND QT_MOC_HEADERS)

source_group(
  "ProjectDataModel\\StimPlanModel"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake)
