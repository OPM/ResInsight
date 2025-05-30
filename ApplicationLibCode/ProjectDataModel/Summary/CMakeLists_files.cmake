set(SOURCE_GROUP_HEADER_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimAsciiDataCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimFileSummaryCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimGridSummaryCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCaseMainCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsemble.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurvesData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAppearanceCalculator.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAutoName.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotAxisFormatter.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCrossPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTimeAxisProperties.h
    ${CMAKE_CURRENT_LIST_DIR}/RimObservedSummaryData.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryObservedDataFile.h
    ${CMAKE_CURRENT_LIST_DIR}/RimObservedEclipseUserData.h
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
    ${CMAKE_CURRENT_LIST_DIR}/RimDeltaSummaryCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimDeltaSummaryEnsemble.h
    ${CMAKE_CURRENT_LIST_DIR}/RimObjectiveFunction.h
    ${CMAKE_CURRENT_LIST_DIR}/RimObjectiveFunctionTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotManager.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryMultiPlot.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryDataSourceStepping.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryNameHelper.h
    ${CMAKE_CURRENT_LIST_DIR}/RimMultipleSummaryPlotNameHelper.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryAddressCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryMultiPlotCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotControls.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveInfoTextProvider.h
    ${CMAKE_CURRENT_LIST_DIR}/RimRftCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimCsvSummaryCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTable.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTableCollection.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTableTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryDeclineCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryRegressionAnalysisCurve.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryAddressSelector.h
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCrossPlotStatisticsCase.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsembleTools.h
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotReadOut.h
)

set(SOURCE_GROUP_SOURCE_FILES
    ${CMAKE_CURRENT_LIST_DIR}/RimAsciiDataCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimFileSummaryCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimGridSummaryCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCaseMainCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsemble.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurvesData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAppearanceCalculator.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveAutoName.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotAxisFormatter.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCurveCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryCrossPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTimeAxisProperties.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimObservedSummaryData.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryObservedDataFile.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimObservedEclipseUserData.cpp
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
    ${CMAKE_CURRENT_LIST_DIR}/RimDeltaSummaryCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimDeltaSummaryEnsemble.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimObjectiveFunction.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimObjectiveFunctionTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotManager.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryMultiPlot.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryDataSourceStepping.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryNameHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimMultipleSummaryPlotNameHelper.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryAddressCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryMultiPlotCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotControls.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCurveInfoTextProvider.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimRftCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimCsvSummaryCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTable.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTableCollection.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryTableTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryDeclineCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryRegressionAnalysisCurve.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryAddressSelector.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimEnsembleCrossPlotStatisticsCase.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryEnsembleTools.cpp
    ${CMAKE_CURRENT_LIST_DIR}/RimSummaryPlotReadOut.cpp
)

list(APPEND CODE_HEADER_FILES ${SOURCE_GROUP_HEADER_FILES})
list(APPEND CODE_SOURCE_FILES ${SOURCE_GROUP_SOURCE_FILES})
