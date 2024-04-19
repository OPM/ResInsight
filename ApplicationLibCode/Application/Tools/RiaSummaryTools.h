/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
//
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
//
//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
//
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html>
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RiaDateTimeDefines.h"
#include "RimObservedDataCollection.h"

#include <QString>
#include <QStringList>

#include <vector>

class RifEclipseSummaryAddress;
class RimSummaryPlot;
class RimSummaryMultiPlot;
class RimSummaryMultiPlotCollection;
class RimSummaryCaseMainCollection;
class RimSummaryCase;
class RimSummaryCaseCollection;
class RimSummaryTable;
class RimSummaryTableCollection;
class RimObservedDataCollection;
class RimSummaryCurve;
class RimUserDefinedCalculation;

namespace caf
{
class PdmObject;
class PdmOptionItemInfo;
} // namespace caf

//==================================================================================================
//
//==================================================================================================
class RiaSummaryTools
{
public:
    static RimSummaryCaseMainCollection*  summaryCaseMainCollection();
    static RimSummaryMultiPlotCollection* summaryMultiPlotCollection();
    static RimObservedDataCollection*     observedDataCollection();

    static std::vector<RimSummaryCase*> singleTopLevelSummaryCases();

    static void notifyCalculatedCurveNameHasChanged( int calculationId, const QString& currentCurveName );

    static RimSummaryPlot*                parentSummaryPlot( caf::PdmObject* object );
    static RimSummaryMultiPlot*           parentSummaryMultiPlot( caf::PdmObject* object );
    static RimSummaryMultiPlotCollection* parentSummaryPlotCollection( caf::PdmObject* object );

    static RimSummaryTable*           parentSummaryTable( caf::PdmObject* object );
    static RimSummaryTableCollection* parentSummaryTableCollection( caf::PdmObject* object );

    static bool hasAccumulatedData( const RifEclipseSummaryAddress& address );
    static void getSummaryCasesAndAddressesForCalculation( int                                    id,
                                                           std::vector<RimSummaryCase*>&          cases,
                                                           std::vector<RifEclipseSummaryAddress>& addresses );

    static std::pair<std::vector<time_t>, std::vector<double>> resampledValuesForPeriod( const RifEclipseSummaryAddress& address,
                                                                                         const std::vector<time_t>&      timeSteps,
                                                                                         const std::vector<double>&      values,
                                                                                         RiaDefines::DateTimePeriod      period );

    static RimSummaryCase*           summaryCaseById( int caseId );
    static RimSummaryCaseCollection* ensembleById( int ensembleId );

    static QList<caf::PdmOptionItemInfo> optionsForAllSummaryCases();
    static QList<caf::PdmOptionItemInfo> optionsForSummaryCases( const std::vector<RimSummaryCase*>& cases );

    static void copyCurveDataSources( RimSummaryCurve& curve, const RimSummaryCurve& otherCurve );
    static void copyCurveAxisData( RimSummaryCurve& curve, const RimSummaryCurve& otherCurve );

    static void reloadSummaryCase( RimSummaryCase* summaryCase );

private:
    static void updateRequiredCalculatedCurves( RimSummaryCase* sourceSummaryCase );
    static bool isCalculationRequired( const RimUserDefinedCalculation* summaryCalculation, const RimSummaryCase* summaryCase );
};
