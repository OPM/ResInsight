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
#include "RiaPreferences.h"
#include "RifSummaryReaderInterface.h"
#include "RimSummaryCase.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* RicSummaryPlotFeatureImpl::addDefaultCurveToPlot(RimSummaryPlot* plot, RimSummaryCase* summaryCase)
{
    if (plot)
    {
        RifEclipseSummaryAddress defaultAddressToUse;

        QString curvesTextFilter = RiaApplication::instance()->preferences()->defaultSummaryCurvesTextFilter;
        QStringList curveFilters = curvesTextFilter.split(";", QString::SkipEmptyParts);

        if ( curveFilters.size() )
        {
            const std::set<RifEclipseSummaryAddress>&  addrs = summaryCase->summaryReader()->allResultAddresses();

            for ( const auto & addr : addrs )
            {
                const QString& filter =  curveFilters[0];
                {
                    if ( addr.isUiTextMatchingFilterText(filter) )
                    {
                        defaultAddressToUse = addr;
                    }
                }
            }
        }

        RimSummaryCurve* newCurve = new RimSummaryCurve();

        // Use same counting as RicNewSummaryEnsembleCurveSetFeature::onActionTriggered
        cvf::Color3f curveColor = RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f(plot->singleColorCurveCount());
        newCurve->setColor(curveColor);

        plot->addCurveNoUpdate(newCurve);

        if (summaryCase)
        {
            newCurve->setSummaryCaseY(summaryCase);
        }

        newCurve->setSummaryAddressYAndApplyInterpolation(defaultAddressToUse);

        return newCurve;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot(RimSummaryPlot* plot, RimSummaryCase* summaryCase)
{
    std::vector<RimSummaryCurve*> defaultCurves;

    QString curvesTextFilter = RiaApplication::instance()->preferences()->defaultSummaryCurvesTextFilter;
    QStringList curveFilters = curvesTextFilter.split(";", QString::SkipEmptyParts);

    const std::set<RifEclipseSummaryAddress>&  addrs = summaryCase->summaryReader()->allResultAddresses();
    std::vector<RifEclipseSummaryAddress> curveAddressesToUse;

    for (const auto & addr : addrs)
    {
        for (const QString& filter: curveFilters)
        {
            if ( addr.isUiTextMatchingFilterText(filter) )
            {
                curveAddressesToUse.push_back(addr); 
            }
        }
    }

    for (const auto & addr : curveAddressesToUse)
    {
        RimSummaryCurve* newCurve = new RimSummaryCurve();
        plot->addCurveNoUpdate(newCurve);
        if (summaryCase)
        {
            newCurve->setSummaryCaseY(summaryCase);
        }
        newCurve->setSummaryAddressYAndApplyInterpolation(addr);
        defaultCurves.push_back(newCurve);
    }

    return defaultCurves;
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

        std::vector<RimSummaryCurve*>  curves = RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot(plot, summaryCase);

        plot->applyDefaultCurveAppearances();
        plot->loadDataAndUpdate();

        summaryPlotCollection->updateConnectedEditors();

        caf::PdmObject* itemToSelect = plot;
        if (curves.size()) itemToSelect = curves[0];

        RiuPlotMainWindowTools::setExpanded(itemToSelect);
        RiuPlotMainWindowTools::selectAsCurrentItem(itemToSelect);
    }
}


