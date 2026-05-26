set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationMatrixPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewParameterResultCrossPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationReportPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateEnsembleFromFilteredCasesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateHistogramForEnsembleParameterFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateHistogramForSummaryVectorFeature.cpp
)

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
