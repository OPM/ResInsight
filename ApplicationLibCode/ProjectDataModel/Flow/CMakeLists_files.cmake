set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimFlowDiagSolution.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFlowPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellAllocationPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimTotalWellAllocationPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimTofAccumulatedPhaseFractionsPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellFlowRateCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellAllocationPlotLegend.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFlowCharacteristicsPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellRftPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPltPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDataSourceForRftPlt.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellPlotTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellRftEnsembleCurveSet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellDistributionPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellDistributionPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellAllocationOverTimePlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellAllocationTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimWellConnectivityTable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFlowDiagnosticsTools.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
