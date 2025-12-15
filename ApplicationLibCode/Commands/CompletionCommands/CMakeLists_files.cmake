set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicEditPerforationCollectionFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicExportFishbonesLateralsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicFishbonesCreateHelper.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsAtMeasuredDepthFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewFishbonesSubsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewPerforationIntervalAtMeasuredDepthFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewValveFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewValveTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewValveAtMeasuredDepthFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteValveTemplateFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicWellPathImportPerforationIntervalsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewStimPlanModelPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportEnsembleFractureStatisticsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicImportValveTemplatesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDiameterRoughnessIntervalFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteDiameterRoughnessIntervalFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCustomSegmentIntervalFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicDeleteCustomSegmentIntervalFeature.h
)

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

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
