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
#include "RimSummaryCase.h"

#include "RiuPlotMainWindowTools.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaPreferences.h"
#include "RiaEclipseFileNameTools.h"

#include "RifSummaryReaderInterface.h"

#include "RicImportGeneralDataFeature.h"

#include <QStringList>

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

    return addCurvesFromAddressFiltersToPlot(curveFilters,  plot, summaryCase, false);
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

    if (summaryPlotCollection && summaryCase && !RiaApplication::instance()->preferences()->defaultSummaryCurvesTextFilter().isEmpty())
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotFeatureImpl::createSummaryPlotFromCommandLine(const QStringList & arguments)
{
    RimSummaryPlot* plot = RicSummaryPlotFeatureImpl::createSummaryPlotFromArgumentLine(arguments);
    return plot;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicSummaryPlotFeatureImpl::createSummaryPlotFromArgumentLine(const QStringList & arguments)
{
    // Split arguments in options, vectors and filenames

    QStringList options;
    QStringList summaryAddressFilters;
    QStringList summaryFiles;


    for (int optionIdx = 0; optionIdx < arguments.size(); ++optionIdx)
    {
        if (arguments[optionIdx].startsWith("-"))
        {
            if (arguments[optionIdx] == "-help")
            {
                RiaApplication::instance()->showFormattedTextInMessageBoxOrConsole(
                    "The --summaryplot option has the following syntax:\n"
                    "\n"
                    "[<plotOptions>] <eclipsesummaryvectors> [<eclipsedatafiles>]\n"
                    "\n"
                    "It Creates a summary plot using all the <eclipsedatafiles>, and all the summary vectors defined in <eclipsesummaryvectors>.\n"
                    "The <eclipsesummaryvectors> has the syntax <vectorname>[:<item>[:<subitem>[:i,j,k]]] and can be repeated.\n"
                    "Wildcards can also be used, eg. \"WOPT:*\" to select the total oil production from all the wells.\n"
                    "The <eclipsedatafiles> can be written with or without extension. Only the corresponding SMSPEC file will be opened for each case.\n"
                    "\n"
                    "The summary plot options are: \n"
                    "  -help\t Show this help text and ignore the rest of the options.\n"
                    "  -nl\t Omit legend in plot.\n"
                    "  -h\t Include history vectors. Will be read from the summary file if the vectors exist.\n" 
                    "    \t Only history vectors from the first summary case in the project will be included.\n"
                    "  -n\t Scale all curves into the range 0.0-1.0."
                );
                return nullptr;
            }
            options.push_back(arguments[optionIdx]);
        }
        else 
        {
            RiaEclipseFileNameTools nameTool(arguments[optionIdx]);
            QString smSpecFileName = nameTool.findRelatedSummarySpecFile();
            if (smSpecFileName != "")
            {
                summaryFiles.push_back(smSpecFileName);
            }
            else
            {
                summaryAddressFilters.push_back(arguments[optionIdx]) ;
            }
        }
    }

    if ( summaryAddressFilters.empty() )
    {
        RiaLogging::error("Needs at least one summary vector to create a plot.");
    }
    
    if ( summaryFiles.size() )
    {
        RicImportGeneralDataFeature::OpenCaseResults results =
            RicImportGeneralDataFeature::openEclipseFilesFromFileNames(summaryFiles, false);
    }
    bool hideLegend = options.contains("-nl");
    bool addHistoryCurves = options.contains("-h");
    bool isNormalizedY = options.contains("-n");

    std::vector<RimSummaryCase*> summaryCasesToUse = RiaApplication::instance()->project()->allSummaryCases();

    if ( summaryCasesToUse.size() )
    {
        RimSummaryPlotCollection* sumPlotColl = RiaApplication::instance()->project()->mainPlotCollection()->summaryPlotCollection();
        RimSummaryPlot* newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();

        for (RimSummaryCase* sumCase : summaryCasesToUse)
        {
            RicSummaryPlotFeatureImpl::addCurvesFromAddressFiltersToPlot(summaryAddressFilters, newPlot, sumCase, addHistoryCurves);
            addHistoryCurves = false;
        }

        newPlot->showLegend(!hideLegend);
        newPlot->setNormalizationEnabled(isNormalizedY);

        newPlot->applyDefaultCurveAppearances();
        newPlot->loadDataAndUpdate();

        sumPlotColl->updateConnectedEditors();

        RiuPlotMainWindowTools::setExpanded(newPlot);
        RiuPlotMainWindowTools::selectAsCurrentItem(newPlot);
    }
    else
    {
        RiaLogging::error("Needs at least one summary case to create a plot.");
    }


    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryCurve*> RicSummaryPlotFeatureImpl::addCurvesFromAddressFiltersToPlot(const QStringList& curveFilters, 
                                                                                           RimSummaryPlot* plot, 
                                                                                           RimSummaryCase* summaryCase, 
                                                                                           bool addHistoryCurves)
{
    std::vector<RimSummaryCurve*> createdCurves;

    const std::set<RifEclipseSummaryAddress>&  addrs = summaryCase->summaryReader()->allResultAddresses();
    std::vector<RifEclipseSummaryAddress> curveAddressesToUse;
    int curveFilterCount = curveFilters.size();

    std::vector<bool> usedFilters(curveFilterCount, false);

    for (const auto & addr : addrs)
    {
        for (int cfIdx = 0 ; cfIdx < curveFilterCount ;  ++cfIdx)
        {
            if ( addr.isUiTextMatchingFilterText( curveFilters[cfIdx]) )
            {
                curveAddressesToUse.push_back(addr); 
                usedFilters[cfIdx] = true;
            }
        }
    }

    for (int cfIdx = 0 ; cfIdx < curveFilterCount ;  ++cfIdx)
    {
        if (!usedFilters[cfIdx])
        {
            RiaLogging::warning("Vector filter \"" + curveFilters[cfIdx] +   "\" did not match anything in case: \"" + summaryCase->caseName() + "\"");
        }
    }

    if (addHistoryCurves)
    {
        std::vector<RifEclipseSummaryAddress> historyAddressesToUse;
        for (RifEclipseSummaryAddress historyAddr : curveAddressesToUse)
        {
            historyAddr.setQuantityName(historyAddr.quantityName() + "H");
            if (addrs.count(historyAddr))
            {
                historyAddressesToUse.push_back(historyAddr);
            }
        }
        curveAddressesToUse.insert( curveAddressesToUse.end(), historyAddressesToUse.begin(), historyAddressesToUse.end() );
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
        createdCurves.push_back(newCurve);
    }

    return createdCurves;
}
