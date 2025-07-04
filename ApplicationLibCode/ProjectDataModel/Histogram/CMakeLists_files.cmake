set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleParameterHistogramDataSource.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleSummaryVectorHistogramDataSource.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGridStatisticsHistogramDataSource.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramCurveCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramDataSource.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleFractureHistogramDataSource.h
)

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

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
