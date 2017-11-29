
# Use this workaround until we're on 2.8.3 on all platforms and can use CMAKE_CURRENT_LIST_DIR directly 
if (${CMAKE_VERSION} VERSION_GREATER "2.8.2")
    set(CEE_CURRENT_LIST_DIR  ${CMAKE_CURRENT_LIST_DIR}/)
endif()

set (SOURCE_GROUP_HEADER_FILES
${CEE_CURRENT_LIST_DIR}RimAsciiDataCurve.h
${CEE_CURRENT_LIST_DIR}RimFileSummaryCase.h
${CEE_CURRENT_LIST_DIR}RimGridSummaryCase.h
${CEE_CURRENT_LIST_DIR}RimSummaryCase.h
${CEE_CURRENT_LIST_DIR}RimSummaryCaseMainCollection.h
${CEE_CURRENT_LIST_DIR}RimSummaryCaseCollection.h
${CEE_CURRENT_LIST_DIR}RimSummaryCurve.h
${CEE_CURRENT_LIST_DIR}RimSummaryCurveAppearanceCalculator.h
${CEE_CURRENT_LIST_DIR}RimSummaryCurveAutoName.h
${CEE_CURRENT_LIST_DIR}RimSummaryCurveFilter.h
${CEE_CURRENT_LIST_DIR}RimSummaryCurvesCalculator.h
${CEE_CURRENT_LIST_DIR}RimSummaryFilter.h
${CEE_CURRENT_LIST_DIR}RimSummaryCurveCollection.h
${CEE_CURRENT_LIST_DIR}RimSummaryPlot.h
${CEE_CURRENT_LIST_DIR}RimSummaryPlotCollection.h
${CEE_CURRENT_LIST_DIR}RimSummaryCrossPlotCollection.h
${CEE_CURRENT_LIST_DIR}RimSummaryTimeAxisProperties.h
${CEE_CURRENT_LIST_DIR}RimSummaryAxisProperties.h
${CEE_CURRENT_LIST_DIR}RimObservedData.h
${CEE_CURRENT_LIST_DIR}RimObservedDataCollection.h
${CEE_CURRENT_LIST_DIR}RimSummaryObservedDataFile.h
${CEE_CURRENT_LIST_DIR}RimObservedEclipseUserData.h
${CEE_CURRENT_LIST_DIR}RimCalculatedSummaryCase.h
${CEE_CURRENT_LIST_DIR}RimCalculatedSummaryCurveReader.h
${CEE_CURRENT_LIST_DIR}RimSummaryAddress.h
${CEE_CURRENT_LIST_DIR}RimSummaryCrossPlot.h
${CEE_CURRENT_LIST_DIR}RimSummaryPlotSourceStepping.h
${CEE_CURRENT_LIST_DIR}RimCsvUserData.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CEE_CURRENT_LIST_DIR}RimAsciiDataCurve.cpp
${CEE_CURRENT_LIST_DIR}RimFileSummaryCase.cpp
${CEE_CURRENT_LIST_DIR}RimGridSummaryCase.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCase.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCaseMainCollection.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCaseCollection.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCurve.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCurveAppearanceCalculator.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCurveAutoName.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCurveFilter.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCurvesCalculator.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryFilter.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCurveCollection.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryPlot.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryPlotCollection.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCrossPlotCollection.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryTimeAxisProperties.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryAxisProperties.cpp
${CEE_CURRENT_LIST_DIR}RimObservedData.cpp
${CEE_CURRENT_LIST_DIR}RimObservedDataCollection.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryObservedDataFile.cpp
${CEE_CURRENT_LIST_DIR}RimObservedEclipseUserData.cpp
${CEE_CURRENT_LIST_DIR}RimCalculatedSummaryCase.cpp
${CEE_CURRENT_LIST_DIR}RimCalculatedSummaryCurveReader.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryAddress.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryCrossPlot.cpp
${CEE_CURRENT_LIST_DIR}RimSummaryPlotSourceStepping.cpp
${CEE_CURRENT_LIST_DIR}RimCsvUserData.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Summary" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CEE_CURRENT_LIST_DIR}CMakeLists_files.cmake )
