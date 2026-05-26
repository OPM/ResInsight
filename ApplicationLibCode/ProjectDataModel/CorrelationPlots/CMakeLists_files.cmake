set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimAbstractCorrelationPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCorrelationPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCorrelationMatrixPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterResultCrossPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimParameterRftCrossPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCorrelationPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCorrelationReportPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRftCorrelationReportPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRftTornadoPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCorrelationBarChartTools.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
