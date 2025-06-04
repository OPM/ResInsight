set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlotCollection.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimHistogramMultiPlotCollection.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
