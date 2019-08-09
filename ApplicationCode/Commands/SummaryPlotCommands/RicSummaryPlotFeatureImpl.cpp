/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicSummaryPlotFeatureImpl.h"

#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimProject.h"
#include "RimMainPlotCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicSummaryPlotFeatureImpl::addDefaultCurveToPlot(RimSummaryPlot* plot, RimSummaryCase* summaryCase)
{
    if (plot)
    {
        RimSummaryCurve* newCurve = new RimSummaryCurve();

        // Use same counting as RicNewSummaryEnsembleCurveSetFeature::onActionTriggered
        cvf::Color3f curveColor = RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f(plot->singleColorCurveCount());
        newCurve->setColor(curveColor);

        plot->addCurveNoUpdate(newCurve);

        if (summaryCase)
        {
            newCurve->setSummaryCaseY(summaryCase);
        }

        newCurve->setSummaryAddressYAndApplyInterpolation(RifEclipseSummaryAddress::fieldAddress("FOPT"));

        return newCurve;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotFeatureImpl::ensureAtLeastOnePlot(RimSummaryPlotCollection* summaryPlotCollection, RimSummaryCase* summaryCase)
{
    if (summaryPlotCollection && summaryCase)
    {
        if (summaryPlotCollection->summaryPlots.empty())
        {
            createDefaultSummaryPlot(summaryCase);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSummaryPlotFeatureImpl::createDefaultSummaryPlot( RimSummaryCase* summaryCase)
{
    RimSummaryPlotCollection* summaryPlotCollection = RiaApplication::instance()->project()->mainPlotCollection->summaryPlotCollection();

    if (summaryPlotCollection && summaryCase)
    {
        auto plot = summaryPlotCollection->createSummaryPlotWithAutoTitle();

        auto curve = RicSummaryPlotFeatureImpl::addDefaultCurveToPlot(plot, summaryCase);
        plot->loadDataAndUpdate();

        summaryPlotCollection->updateConnectedEditors();

        RiuPlotMainWindowTools::setExpanded(curve);
        RiuPlotMainWindowTools::selectAsCurrentItem(curve);
    }
}


