
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()


list(APPEND CODE_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RimCaseCollection.h
${CEE_CURRENT_LIST_DIR}RimCellFilter.h
${CEE_CURRENT_LIST_DIR}RimCellPropertyFilter.h
${CEE_CURRENT_LIST_DIR}RimCellPropertyFilterCollection.h
${CEE_CURRENT_LIST_DIR}RimCellRangeFilter.h
${CEE_CURRENT_LIST_DIR}RimCellRangeFilterCollection.h
${CEE_CURRENT_LIST_DIR}RimDefines.h
${CEE_CURRENT_LIST_DIR}RimLegendConfig.h
${CEE_CURRENT_LIST_DIR}RimProject.h
${CEE_CURRENT_LIST_DIR}RimCase.h
${CEE_CURRENT_LIST_DIR}RimIdenticalGridCaseGroup.h
${CEE_CURRENT_LIST_DIR}RimInputProperty.h
${CEE_CURRENT_LIST_DIR}RimInputPropertyCollection.h
${CEE_CURRENT_LIST_DIR}RimInputCase.h
${CEE_CURRENT_LIST_DIR}RimResultCase.h
${CEE_CURRENT_LIST_DIR}RimReservoirView.h
${CEE_CURRENT_LIST_DIR}RimResultDefinition.h
${CEE_CURRENT_LIST_DIR}RimResultSlot.h
${CEE_CURRENT_LIST_DIR}RimCellEdgeResultSlot.h
${CEE_CURRENT_LIST_DIR}RimWell.h
${CEE_CURRENT_LIST_DIR}RimWellCollection.h
${CEE_CURRENT_LIST_DIR}RimScriptCollection.h
${CEE_CURRENT_LIST_DIR}RimStatisticsCase.h
${CEE_CURRENT_LIST_DIR}RimStatisticsCaseCollection.h
${CEE_CURRENT_LIST_DIR}RimCalcScript.h
${CEE_CURRENT_LIST_DIR}RimExportInputPropertySettings.h
${CEE_CURRENT_LIST_DIR}RimBinaryExportSettings.h
${CEE_CURRENT_LIST_DIR}Rim3dOverlayInfoConfig.h
${CEE_CURRENT_LIST_DIR}RimUiTreeModelPdm.h
${CEE_CURRENT_LIST_DIR}RimUiTreeView.h
${CEE_CURRENT_LIST_DIR}RimReservoirCellResultsCacher.h
${CEE_CURRENT_LIST_DIR}RimStatisticsCaseEvaluator.h
${CEE_CURRENT_LIST_DIR}RimMimeData.h
)

list(APPEND CODE_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RimCaseCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCellFilter.cpp
${CEE_CURRENT_LIST_DIR}RimCellPropertyFilter.cpp
${CEE_CURRENT_LIST_DIR}RimCellPropertyFilterCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCellRangeFilter.cpp
${CEE_CURRENT_LIST_DIR}RimCellRangeFilterCollection.cpp
${CEE_CURRENT_LIST_DIR}RimDefines.cpp
${CEE_CURRENT_LIST_DIR}RimLegendConfig.cpp
${CEE_CURRENT_LIST_DIR}RimProject.cpp
${CEE_CURRENT_LIST_DIR}RimCase.cpp
${CEE_CURRENT_LIST_DIR}RimIdenticalGridCaseGroup.cpp
${CEE_CURRENT_LIST_DIR}RimInputProperty.cpp
${CEE_CURRENT_LIST_DIR}RimInputPropertyCollection.cpp
${CEE_CURRENT_LIST_DIR}RimInputCase.cpp
${CEE_CURRENT_LIST_DIR}RimResultCase.cpp
${CEE_CURRENT_LIST_DIR}RimReservoirView.cpp
${CEE_CURRENT_LIST_DIR}RimResultDefinition.cpp
${CEE_CURRENT_LIST_DIR}RimResultSlot.cpp
${CEE_CURRENT_LIST_DIR}RimCellEdgeResultSlot.cpp
${CEE_CURRENT_LIST_DIR}RimWell.cpp
${CEE_CURRENT_LIST_DIR}RimWellCollection.cpp
${CEE_CURRENT_LIST_DIR}RimScriptCollection.cpp
${CEE_CURRENT_LIST_DIR}RimStatisticsCase.cpp
${CEE_CURRENT_LIST_DIR}RimStatisticsCaseCollection.cpp
${CEE_CURRENT_LIST_DIR}RimCalcScript.cpp
${CEE_CURRENT_LIST_DIR}RimExportInputPropertySettings.cpp
${CEE_CURRENT_LIST_DIR}RimBinaryExportSettings.cpp
${CEE_CURRENT_LIST_DIR}Rim3dOverlayInfoConfig.cpp
${CEE_CURRENT_LIST_DIR}RimUiTreeModelPdm.cpp
${CEE_CURRENT_LIST_DIR}RimUiTreeView.cpp
${CEE_CURRENT_LIST_DIR}RimReservoirCellResultsCacher.cpp
${CEE_CURRENT_LIST_DIR}RimStatisticsCaseEvaluator.cpp
${CEE_CURRENT_LIST_DIR}RimMimeData.cpp
)


