set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicEditPerforationCollectionFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesLateralsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicFishbonesCreateHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsAtMeasuredDepthFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalAtMeasuredDepthFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewValveFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewValveTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewValveAtMeasuredDepthFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteValveTemplateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathImportPerforationIntervalsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanModelPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportEnsembleFractureStatisticsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicImportValveTemplatesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDiameterRoughnessIntervalFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteDiameterRoughnessIntervalFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCustomSegmentIntervalFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteCustomSegmentIntervalFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
