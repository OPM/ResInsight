set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationReportPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationMatrixPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewParameterResultCrossPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateEnsembleFromFilteredCasesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateHistogramForEnsembleParameterFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateHistogramForSummaryVectorFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationMatrixPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewParameterResultCrossPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewCorrelationReportPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateEnsembleFromFilteredCasesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateHistogramForEnsembleParameterFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicCreateHistogramForSummaryVectorFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
