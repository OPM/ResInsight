
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelTemplate.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModel.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelTemplateCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelElasticPropertyCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelLayerCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelPropertyCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelPropertyCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelStressCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelWellLogCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RimElasticProperties.h
${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScaling.h
${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScalingCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimFaciesProperties.h
${CMAKE_CURRENT_LIST_DIR}/RimNonNetLayers.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelTemplate.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModel.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelTemplateCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelElasticPropertyCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelLayerCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelStressCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFractureModelWellLogCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RimElasticProperties.cpp
${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScaling.cpp
${CMAKE_CURRENT_LIST_DIR}/RimElasticPropertyScalingCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFaciesProperties.cpp
${CMAKE_CURRENT_LIST_DIR}/RimNonNetLayers.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

list(APPEND QT_MOC_HEADERS
)

source_group( "ProjectDataModel\\StimPlanModel" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
