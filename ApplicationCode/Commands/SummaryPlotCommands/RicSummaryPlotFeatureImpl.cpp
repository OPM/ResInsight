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
                    "[<plotOptions>] <eclipsesummaryvectorfilters> [<eclipsedatafiles>]\n"
                    "\n"
                    "It Creates one summary plot using all the <eclipsedatafiles> for each of the the summary vectors matched by the <eclipsesummaryvectorfilters>.\n"
                    "The <eclipsesummaryvectorfilters> has the syntax <vectorname>[:<item>[:<subitem>[:i,j,k]]] and can be repeated.\n"
                    "Wildcards can also be used, eg. \"WOPT:*\" to select the total oil production from all the wells.\n"
                    "The <eclipsedatafiles> can be written with or without extension. Only the corresponding SMSPEC file will be opened for each case.\n"
                    "\n"
                    "The summary plot options are: \n"
                    "  -help\t Show this help text and ignore the rest of the options.\n"
                    "  -h\t Include history vectors. Will be read from the summary file if the vectors exist.\n" 
                    "    \t Only history vectors from the first summary case in the project will be included.\n"
                    "  -nl\t Omit legend in plot.\n"
                    "  -s\t Create only one plot including all the defined vectors and cases."
                    "  -n\t Scale all curves into the range 0.0-1.0. Useful when using -s"
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
    bool isSinglePlot = options.contains("-s");

    std::vector<RimSummaryCase*> summaryCasesToUse = RiaApplication::instance()->project()->allSummaryCases();

    if ( summaryCasesToUse.size() )
    {
        RimSummaryPlotCollection* sumPlotColl = RiaApplication::instance()->project()->mainPlotCollection()->summaryPlotCollection();
        if ( isSinglePlot )
        {
            RimSummaryPlot* newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();

            for ( RimSummaryCase* sumCase : summaryCasesToUse )
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
        else // Multiplot, one for each separate summary address
        {
            std::set<RifEclipseSummaryAddress> filteredAdressesFromCases;
            RimSummaryPlot* lastPlotCreated = nullptr;

            for ( RimSummaryCase* sumCase : summaryCasesToUse )
            {
                const std::set<RifEclipseSummaryAddress>&  addrs = sumCase->summaryReader()->allResultAddresses();
                std::vector<bool> usedFilters;

                filteredSummaryAdressesFromCase(summaryAddressFilters, addrs, &filteredAdressesFromCases, &usedFilters);

                for (int cfIdx = 0 ; cfIdx < usedFilters.size() ;  ++cfIdx)
                {
                    if (!usedFilters[cfIdx])
                    {
                        RiaLogging::warning("Vector filter \"" + summaryAddressFilters[cfIdx] +   "\" did not match anything in case: \"" + sumCase->caseName() + "\"");
                    }
                }
            }

            for (const auto & addr : filteredAdressesFromCases)
            {
                std::vector<RimSummaryCurve*> createdCurves;
                bool isFirstCase = true;
                for ( RimSummaryCase* sumCase : summaryCasesToUse )
                {
                    const std::set<RifEclipseSummaryAddress>&  addrs = sumCase->summaryReader()->allResultAddresses();
                    if (addrs.count(addr))
                    {
                        RimSummaryCurve* newCurve = new RimSummaryCurve();
                        newCurve->setSummaryCaseY(sumCase);
                        newCurve->setSummaryAddressYAndApplyInterpolation(addr);
                        createdCurves.push_back(newCurve);

                        if ( addHistoryCurves && isFirstCase)
                        {
                            RifEclipseSummaryAddress historyAddr = addr;
                            historyAddr.setQuantityName(historyAddr.quantityName() + "H");
                            if ( addrs.count(historyAddr) )
                            {
                                RimSummaryCurve* historyCurve = new RimSummaryCurve();
                                historyCurve->setSummaryCaseY(sumCase);
                                historyCurve->setSummaryAddressYAndApplyInterpolation(historyAddr);
                                createdCurves.push_back(historyCurve);
                            }
                        }
                    }

                    isFirstCase = false;
                }

                if ( createdCurves.size() )
                {
                    RimSummaryPlot* newPlot = sumPlotColl->createSummaryPlotWithAutoTitle();
                    for ( auto curve : createdCurves )
                    {
                        newPlot->addCurveNoUpdate(curve);
                    }

                    newPlot->showLegend(!hideLegend);
                    newPlot->setNormalizationEnabled(isNormalizedY);

                    newPlot->applyDefaultCurveAppearances();
                    newPlot->loadDataAndUpdate();
                    lastPlotCreated = newPlot;
                }
            }

            sumPlotColl->updateConnectedEditors();
            if ( lastPlotCreated )
            {
                RiuPlotMainWindowTools::setExpanded(lastPlotCreated);
                RiuPlotMainWindowTools::selectAsCurrentItem(lastPlotCreated);
            }
        }
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

    std::set<RifEclipseSummaryAddress> curveAddressesToUse;

    const std::set<RifEclipseSummaryAddress>&  addrs = summaryCase->summaryReader()->allResultAddresses();
    std::vector<bool> usedFilters;

    filteredSummaryAdressesFromCase(curveFilters, addrs, &curveAddressesToUse, &usedFilters);

    for (int cfIdx = 0 ; cfIdx < usedFilters.size() ;  ++cfIdx)
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
        curveAddressesToUse.insert( historyAddressesToUse.begin(), historyAddressesToUse.end() );
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

void  RicSummaryPlotFeatureImpl::filteredSummaryAdressesFromCase(const QStringList& curveFilters,
                                                                 const std::set<RifEclipseSummaryAddress>&  allAddressesInCase,
                                                                 std::set<RifEclipseSummaryAddress>* setToInsertFilteredAddressesIn,
                                                                 std::vector<bool>* usedFilters)
{
    int curveFilterCount = curveFilters.size();

    usedFilters->clear();
    usedFilters->resize(curveFilterCount, false);

    for (const auto & addr : allAddressesInCase)
    {
        for (int cfIdx = 0 ; cfIdx < curveFilterCount ;  ++cfIdx)
        {
            if ( addr.isUiTextMatchingFilterText( curveFilters[cfIdx]) )
            {
                setToInsertFilteredAddressesIn->insert(addr); 
                (*usedFilters)[cfIdx] = true;
            }
        }
    }
}