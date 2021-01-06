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

#include <QString>

#include <vector>

class RimSummaryPlotCollection;
class RimSummaryPlot;
class RimSummaryCrossPlot;
class RimSummaryCrossPlotCollection;
class RimSummaryCaseMainCollection;
class RimSummaryCase;

class RifEclipseSummaryAddress;

class QStringList;

namespace caf
{
class PdmObject;
}

//==================================================================================================
//
//==================================================================================================
class RiaSummaryTools
{
public:
    static RimSummaryPlotCollection*      summaryPlotCollection();
    static RimSummaryCrossPlotCollection* summaryCrossPlotCollection();
    static RimSummaryCaseMainCollection*  summaryCaseMainCollection();

    static void notifyCalculatedCurveNameHasChanged( int calculationId, const QString& currentCurveName );

    static RimSummaryPlot*           parentSummaryPlot( caf::PdmObject* object );
    static RimSummaryPlotCollection* parentSummaryPlotCollection( caf::PdmObject* object );

    static RimSummaryCrossPlot*           parentCrossPlot( caf::PdmObject* object );
    static RimSummaryCrossPlotCollection* parentCrossPlotCollection( caf::PdmObject* object );
    static bool                           isSummaryCrossPlot( const RimSummaryPlot* plot );

    static bool hasAccumulatedData( const RifEclipseSummaryAddress& address );
    static void getSummaryCasesAndAddressesForCalculation( int                                    id,
                                                           std::vector<RimSummaryCase*>&          cases,
                                                           std::vector<RifEclipseSummaryAddress>& addresses );

    static QString findSuitableEnsembleName( const QStringList& summaryCaseFileNames );
};
