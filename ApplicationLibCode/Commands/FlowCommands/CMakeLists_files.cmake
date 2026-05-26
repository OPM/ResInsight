set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicAddStoredFlowCharacteristicsPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowWellAllocationPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowFlowCharacteristicsPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAddStoredWellAllocationPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowContributingWellsFromPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowContributingWellsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowContributingWellsFeatureImpl.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPlotProductionRateFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSelectViewUI.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowTotalAllocationDataFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicShowCumulativePhasePlotFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
