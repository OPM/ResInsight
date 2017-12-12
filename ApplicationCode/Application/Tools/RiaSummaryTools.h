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

class RimSummaryPlotCollection;
class RimSummaryPlot;
class RimSummaryCrossPlot;
class RimSummaryCrossPlotCollection;

class QString;

namespace caf {
    class PdmObject;
}

//==================================================================================================
// 
//==================================================================================================
class RiaSummaryTools
{
public:
    static RimSummaryPlotCollection*    summaryPlotCollection();
    static void                         notifyCalculatedCurveNameHasChanged(const QString& previousCurveName,
                                                                            const QString& currentCurveName);

    static RimSummaryPlot*                  parentSummaryPlot(caf::PdmObject* object);
    static RimSummaryPlotCollection*        parentSummaryPlotCollection(caf::PdmObject* object);

    static RimSummaryCrossPlot*             parentCrossPlot(caf::PdmObject* object);
    static RimSummaryCrossPlotCollection*   parentCrossPlotCollection(caf::PdmObject* object);
    static bool                             isSummaryCrossPlot(const RimSummaryPlot* plot);
};
