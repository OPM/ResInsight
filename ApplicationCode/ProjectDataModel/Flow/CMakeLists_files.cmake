
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimFlowDiagSolution.h
${CMAKE_CURRENT_LIST_DIR}/RimFlowPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimWellAllocationPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimTotalWellAllocationPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimTofAccumulatedPhaseFractionsPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimWellFlowRateCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimWellAllocationPlotLegend.h
${CMAKE_CURRENT_LIST_DIR}/RimFlowCharacteristicsPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimWellRftPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimWellPltPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimDataSourceForRftPlt.h
${CMAKE_CURRENT_LIST_DIR}/RimWellPlotTools.h
${CMAKE_CURRENT_LIST_DIR}/RimWellRftEnsembleCurveSet.h
${CMAKE_CURRENT_LIST_DIR}/RimWellDistributionPlot.h
)

set (SOURCE_GROUP_SOURCE_FILES
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
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Flow" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
