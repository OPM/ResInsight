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

#include "RiaSummaryTools.h"

#include "RiaApplication.h"

#include "RifEclipseSummaryAddress.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotCollection* RiaSummaryTools::summaryPlotCollection()
{
    RimProject* project = RiaApplication::instance()->project();

    return project->mainPlotCollection()->summaryPlotCollection();
}

//--------------------------------------------------------------------------------------------------
/// Update the summary curves referencing this curve, as the curve is identified by the name
//--------------------------------------------------------------------------------------------------
void RiaSummaryTools::notifyCalculatedCurveNameHasChanged(const QString& previousCurveName, const QString& currentCurveName)
{
    RimSummaryPlotCollection* summaryPlotColl = RiaSummaryTools::summaryPlotCollection();

    for (RimSummaryPlot* plot : summaryPlotColl->summaryPlots())
    {
        for (RimSummaryCurve* curve : plot->summaryCurves())
        {
            RifEclipseSummaryAddress adr = curve->summaryAddress();
            if (adr.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED)
            {
                if (adr.quantityName() == previousCurveName.toStdString())
                {
                    RifEclipseSummaryAddress updatedAdr = RifEclipseSummaryAddress::calculatedCurveAddress(currentCurveName.toStdString());
                    curve->setSummaryAddress(updatedAdr);
                }
            }
        }
    }
}
