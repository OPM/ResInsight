
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RicAddStoredFlowCharacteristicsPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowWellAllocationPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowFlowCharacteristicsPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicAddStoredWellAllocationPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowContributingWellsFromPlotFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowContributingWellsFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicShowContributingWellsFeatureImpl.h
${CMAKE_CURRENT_LIST_DIR}/RicPlotProductionRateFeature.h
${CMAKE_CURRENT_LIST_DIR}/RicSelectViewUI.h
${CMAKE_CURRENT_LIST_DIR}/RicShowTotalAllocationDataFeature.h
)

set (SOURCE_GROUP_SOURCE_FILES
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
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "CommandFeature\\FlowDiagnostics" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
