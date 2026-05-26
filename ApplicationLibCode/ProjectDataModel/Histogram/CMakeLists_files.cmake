set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleParameterHistogramDataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSummaryVectorHistogramDataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridStatisticsHistogramDataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramCurveCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramDataSource.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleFractureHistogramDataSource.cpp
)

list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
