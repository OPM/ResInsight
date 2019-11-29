
set (SOURCE_GROUP_HEADER_FILES
${CMAKE_CURRENT_LIST_DIR}/RimAsciiDataCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimFileSummaryCase.h
${CMAKE_CURRENT_LIST_DIR}/RimGridSummaryCase.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCase.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCaseMainCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCaseCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurve.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAppearanceCalculator.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAutoName.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotAxisFormatter.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCrossPlotCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryTimeAxisProperties.h
${CMAKE_CURRENT_LIST_DIR}/RimObservedSummaryData.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryObservedDataFile.h
${CMAKE_CURRENT_LIST_DIR}/RimObservedEclipseUserData.h
${CMAKE_CURRENT_LIST_DIR}/RimCalculatedSummaryCase.h
${CMAKE_CURRENT_LIST_DIR}/RimCalculatedSummaryCurveReader.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryAddress.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCrossPlot.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotSourceStepping.h
${CMAKE_CURRENT_LIST_DIR}/RimCsvUserData.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotNameHelper.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveSetCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveSet.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveSetColorManager.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveFilter.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveFilterCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatistics.h
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatisticsCase.h
${CMAKE_CURRENT_LIST_DIR}/RimDerivedEnsembleCase.h
${CMAKE_CURRENT_LIST_DIR}/RimDerivedEnsembleCaseCollection.h
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotFilterTextCurveSetEditor.h
)

set (SOURCE_GROUP_SOURCE_FILES
${CMAKE_CURRENT_LIST_DIR}/RimAsciiDataCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimFileSummaryCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RimGridSummaryCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCaseMainCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCaseCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurve.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAppearanceCalculator.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAutoName.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotAxisFormatter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCrossPlotCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryTimeAxisProperties.cpp
${CMAKE_CURRENT_LIST_DIR}/RimObservedSummaryData.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryObservedDataFile.cpp
${CMAKE_CURRENT_LIST_DIR}/RimObservedEclipseUserData.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCalculatedSummaryCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCalculatedSummaryCurveReader.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryAddress.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryCrossPlot.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotSourceStepping.cpp
${CMAKE_CURRENT_LIST_DIR}/RimCsvUserData.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotNameHelper.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveSetCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveSet.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveSetColorManager.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveFilter.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveFilterCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatistics.cpp
${CMAKE_CURRENT_LIST_DIR}/RimEnsembleStatisticsCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RimDerivedEnsembleCase.cpp
${CMAKE_CURRENT_LIST_DIR}/RimDerivedEnsembleCaseCollection.cpp
${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotFilterTextCurveSetEditor.cpp
)

list(APPEND CODE_HEADER_FILES
${SOURCE_GROUP_HEADER_FILES}
)

list(APPEND CODE_SOURCE_FILES
${SOURCE_GROUP_SOURCE_FILES}
)

source_group( "ProjectDataModel\\Summary" FILES ${SOURCE_GROUP_HEADER_FILES} ${SOURCE_GROUP_SOURCE_FILES} ${CMAKE_CURRENT_LIST_DIR}/CMakeLists_files.cmake )
