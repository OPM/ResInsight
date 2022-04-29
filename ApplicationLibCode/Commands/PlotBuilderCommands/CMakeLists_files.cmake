set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewMultiPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicSummaryPlotBuilder.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryMultiPlotFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryPlotFromDataVectorFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryMultiPlotFromDataVectorFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryPlotFromCurveFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryPlotsForObjectsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryCurvesForObjectsFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryPlotsForSummaryCasesFeature.h
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryCurvesForSummaryCasesFeature.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RicNewMultiPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicSummaryPlotBuilder.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryMultiPlotFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryPlotFromDataVectorFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryMultiPlotFromDataVectorFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicNewSummaryPlotFromCurveFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryPlotsForObjectsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryCurvesForObjectsFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryPlotsForSummaryCasesFeature.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RicAppendSummaryCurvesForSummaryCasesFeature.cpp
)

list(APPEND COMMAND_CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})

list(APPEND COMMAND_CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})

list(APPEND COMMAND_QT_MOC_HEADERS)

source_group(
  "CommandFeature\\PlotBuilder"
  FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES}
        ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake
)
