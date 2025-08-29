set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDefaultHistogramPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewHistogramCurveFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewHistogramMultiPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicPasteHistogramCurveFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewDefaultHistogramPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewHistogramCurveFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewHistogramMultiPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicPasteHistogramCurveFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
