set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotRegressionCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotDataSet.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridCrossPlotTextProvider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSaturationPressurePlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSaturationPressurePlotCollection.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
