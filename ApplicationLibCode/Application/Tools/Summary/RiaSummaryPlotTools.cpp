////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RiaSummaryPlotTools.h"

#include "SummaryPlotCommands/RicNewSummaryEnsembleCurveSetFeature.h"
#include "SummaryPlotCommands/RicSummaryPlotFeatureImpl.h"

#include "RiaPreferencesSummary.h"
#include "RiaSummaryPlotTemplateTools.h"

#include "RifSummaryReaderInterface.h"

#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimPlot.h"
#include "RimSaturationPressurePlot.h"
#include "RimSummaryCase.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuDockWidgetTools.h"
#include "RiuPlotMainWindowTools.h"

namespace RiaSummaryPlotTools
{

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RifEclipseSummaryAddress> addressesForSource( caf::PdmObject* summarySource )
{
    auto ensemble = dynamic_cast<RimSummaryEnsemble*>( summarySource );
    if ( ensemble )
    {
        return ensemble->ensembleSummaryAddresses();
    }

    auto sumCase = dynamic_cast<RimSummaryCase*>( summarySource );
    if ( sumCase )
    {
        auto reader = sumCase ? sumCase->summaryReader() : nullptr;
        if ( reader )
        {
            return reader->allResultAddresses();
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* createCurveSet( RimSummaryEnsemble* ensemble, const RifEclipseSummaryAddress& addr )
{
    auto curveSet = new RimEnsembleCurveSet();

    curveSet->setSummaryEnsemble( ensemble );
    curveSet->setSummaryAddressYAndStatisticsFlag( addr );

    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* createCurve( RimSummaryCase* summaryCase, const RifEclipseSummaryAddress& addr )
{
    auto curve = new RimSummaryCurve();

    curve->setSummaryCaseY( summaryCase );
    curve->setSummaryAddressY( addr );

    size_t pos = addr.vectorName().find( '_' );
    if ( pos != std::string::npos )
    {
        // https://github.com/OPM/ResInsight/issues/12157
        curve->enableVectorNameInCurveName( true );
    }

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlot*> duplicatePlots( const std::vector<RimPlot*>& sourcePlots )
{
    std::vector<RimPlot*> plots;

    for ( auto plot : sourcePlots )
    {
        auto copy = plot->copyObject<RimPlot>();
        {
            // TODO: Workaround for fixing the PdmPointer in RimEclipseResultDefinition
            //    caf::PdmPointer<RimEclipseCase> m_eclipseCase;
            // This PdmPointer must be changed to a ptrField

            auto saturationPressurePlotOriginal = dynamic_cast<RimSaturationPressurePlot*>( plot );
            auto saturationPressurePlotCopy     = dynamic_cast<RimSaturationPressurePlot*>( copy );
            if ( saturationPressurePlotCopy && saturationPressurePlotOriginal )
            {
                RimSaturationPressurePlot::fixPointersAfterCopy( saturationPressurePlotOriginal, saturationPressurePlotCopy );
            }
        }

        plots.push_back( copy );
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSummaryPlot*> duplicateSummaryPlots( const std::vector<RimSummaryPlot*>& sourcePlots )
{
    std::vector<RimSummaryPlot*> plots;

    for ( auto plot : sourcePlots )
    {
        auto copy = plot->copyObject<RimSummaryPlot>();
        if ( copy )
        {
            plots.push_back( copy );
        }
    }

    return plots;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlot* createAndAppendMultiPlot( const std::vector<RimPlot*>& plots )
{
    RimMultiPlotCollection* plotCollection = RimMainPlotCollection::current()->multiPlotCollection();

    auto* plotWindow = new RimMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addMultiPlot( plotWindow );

    appendPlotsToMultiPlot( plotWindow, plots );

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    if ( !plots.empty() )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plots[0] );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow );
    }

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* createAndAppendSummaryMultiPlot( const std::vector<caf::PdmObjectHandle*>& objects )
{
    RimSummaryMultiPlotCollection* plotCollection = RimMainPlotCollection::current()->summaryMultiPlotCollection();

    auto* plotWindow = new RimSummaryMultiPlot;
    plotWindow->setMultiPlotTitle( QString( "Multi Plot %1" ).arg( plotCollection->multiPlots().size() + 1 ) );
    plotWindow->setAsPlotMdiWindow();
    plotCollection->addSummaryMultiPlot( plotWindow );

    plotWindow->handleDroppedObjects( objects );

    plotCollection->updateAllRequiredEditors();
    plotWindow->loadDataAndUpdate();

    if ( plotWindow->summaryPlots().size() == 1 )
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow->summaryPlots()[0] );
        RiuPlotMainWindowTools::setExpanded( plotWindow->summaryPlots()[0] );
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( plotWindow );
        RiuPlotMainWindowTools::setExpanded( plotWindow );
    }

    return plotWindow;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void appendPlotsToMultiPlot( RimMultiPlot* multiPlot, const std::vector<RimPlot*>& plots )
{
    for ( auto plot : plots )
    {
        // Remove the current window controller, as this will be managed by the multi plot
        // This must be done before adding the plot to the multi plot to ensure that the viewer widget is recreated
        plot->revokeMdiWindowStatus();

        multiPlot->addPlot( plot );

        plot->resolveReferencesRecursively();
        plot->setShowWindow( true );

        plot->loadDataAndUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* createAndAppendDefaultSummaryMultiPlot( const std::vector<RimSummaryCase*>&     cases,
                                                             const std::vector<RimSummaryEnsemble*>& ensembles,
                                                             bool                                    skipCreationOfPlotBasedOnPreferences )
{
    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    if ( skipCreationOfPlotBasedOnPreferences && prefs->defaultSummaryPlotType() == RiaPreferencesSummary::DefaultSummaryPlotType::NONE )
        return nullptr;

    if ( prefs->defaultSummaryPlotType() == RiaPreferencesSummary::DefaultSummaryPlotType::PLOT_TEMPLATES )
    {
        RimSummaryMultiPlot* plotToSelect      = nullptr;
        bool                 ensembleTemplates = ( !ensembles.empty() );
        for ( auto& filename : prefs->defaultSummaryPlotTemplates( ensembleTemplates ) )
        {
            plotToSelect = RicSummaryPlotTemplateTools::create( filename, cases, ensembles );
        }

        if ( plotToSelect ) return plotToSelect;

        if ( skipCreationOfPlotBasedOnPreferences )
        {
            return plotToSelect;
        }
    }

    if ( skipCreationOfPlotBasedOnPreferences && prefs->defaultSummaryCurvesTextFilter().trimmed().isEmpty() ) return nullptr;

    auto* plotCollection = RimMainPlotCollection::current()->summaryMultiPlotCollection();

    auto* summaryMultiPlot = new RimSummaryMultiPlot();
    summaryMultiPlot->setAsPlotMdiWindow();
    plotCollection->addSummaryMultiPlot( summaryMultiPlot );

    RimSummaryPlot* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle( true );

    for ( auto sumCase : cases )
    {
        RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot( plot, sumCase );
    }

    for ( auto ensemble : ensembles )
    {
        RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( plot, ensemble );
    }

    plot->applyDefaultCurveAppearances();
    plot->loadDataAndUpdate();

    plotCollection->updateConnectedEditors();

    appendPlotsToSummaryMultiPlot( summaryMultiPlot, { plot } );

    summaryMultiPlot->setDefaultRangeAggregationSteppingDimension();

    plotCollection->updateAllRequiredEditors();
    summaryMultiPlot->loadDataAndUpdate();
    summaryMultiPlot->updateAllRequiredEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );
    if ( !plot->curveSets().empty() )
    {
        RiuPlotMainWindowTools::setExpanded( plot->curveSets().front() );
    }
    else if ( !plot->summaryCurves().empty() )
    {
        RiuPlotMainWindowTools::setExpanded( plot->summaryCurves().front() );
    }

    return summaryMultiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* createAndAppendSingleSummaryMultiPlotNoAutoSettings( RimSummaryPlot* plot )
{
    auto* plotCollection = RimMainPlotCollection::current()->summaryMultiPlotCollection();

    auto* summaryMultiPlot = new RimSummaryMultiPlot();
    summaryMultiPlot->setColumnCount( RiaDefines::ColumnCount::COLUMNS_1 );
    summaryMultiPlot->setRowCount( RiaDefines::RowCount::ROWS_1 );
    summaryMultiPlot->setAsPlotMdiWindow();

    if ( !plot->autoPlotTitle() )
    {
        // Move settings from the single summary plot to the multi plot, and disable auto titles
        summaryMultiPlot->setAutoPlotTitle( false );
        summaryMultiPlot->setAutoSubPlotTitle( false );
        summaryMultiPlot->setMultiPlotTitleVisible( true );
        summaryMultiPlot->setMultiPlotTitle( plot->description() );

        plot->setPlotTitleVisible( false );
        plot->setDescription( "" );
    }

    plotCollection->addSummaryMultiPlot( summaryMultiPlot );

    appendPlotsToSummaryMultiPlot( summaryMultiPlot, { plot } );

    plotCollection->updateAllRequiredEditors();
    summaryMultiPlot->loadDataAndUpdate();
    summaryMultiPlot->updateAllRequiredEditors();

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );

    return summaryMultiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* createAndAppendSummaryMultiPlot( const std::vector<RimSummaryPlot*>& plots )
{
    auto* plotCollection = RimMainPlotCollection::current()->summaryMultiPlotCollection();

    auto* summaryMultiPlot = new RimSummaryMultiPlot();
    summaryMultiPlot->setAsPlotMdiWindow();
    plotCollection->addSummaryMultiPlot( summaryMultiPlot );

    appendPlotsToSummaryMultiPlot( summaryMultiPlot, plots );

    summaryMultiPlot->setDefaultRangeAggregationSteppingDimension();

    plotCollection->updateAllRequiredEditors();
    summaryMultiPlot->loadDataAndUpdate();
    summaryMultiPlot->updateAllRequiredEditors();
    summaryMultiPlot->zoomAll();

    if ( !plots.empty() )
    {
        auto* plot = plots.front();
        if ( !plot->curveSets().empty() )
        {
            RiuPlotMainWindowTools::selectAsCurrentItem( plot->curveSets().front() );
        }
        else if ( !plot->summaryCurves().empty() )
        {
            RiuPlotMainWindowTools::selectAsCurrentItem( plot->summaryCurves().front() );
        }
        else
        {
            RiuPlotMainWindowTools::selectAsCurrentItem( plot );
        }
    }
    else
    {
        RiuPlotMainWindowTools::selectAsCurrentItem( summaryMultiPlot );
    }

    return summaryMultiPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlot* createAndAppendSingleSummaryMultiPlot( RimSummaryPlot* plot )
{
    std::vector<RimSummaryPlot*> plots{ plot };

    return createAndAppendSummaryMultiPlot( plots );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void appendPlotsToSummaryMultiPlot( RimSummaryMultiPlot* multiPlot, const std::vector<RimSummaryPlot*>& plots )
{
    multiPlot->startBatchAddOperation();
    for ( auto plot : plots )
    {
        plot->revokeMdiWindowStatus();

        multiPlot->addPlot( plot );

        plot->resolveReferencesRecursively();
        plot->setShowWindow( true );
    }
    multiPlot->endBatchAddOperation();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* createPlot( const std::vector<RimSummaryCurve*>& summaryCurves )
{
    auto* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle( true );

    for ( auto& curve : summaryCurves )
    {
        plot->addCurveNoUpdate( curve );
    }

    plot->applyDefaultCurveAppearances();

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* createPlot( const std::set<RifEclipseSummaryAddress>& addresses,
                            const std::vector<RimSummaryCase*>&       summaryCases,
                            const std::vector<RimSummaryEnsemble*>&   ensembles )
{
    auto* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle( true );

    appendCurvesToPlot( plot, addresses, summaryCases, ensembles );

    plot->applyDefaultCurveAppearances();

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* createCrossPlot( const std::vector<RiaSummaryCurveAddress>& addresses,
                                 const std::vector<RimSummaryCase*>&        summaryCases,
                                 const std::vector<RimSummaryEnsemble*>&    ensembles )
{
    auto* summaryPlot = new RimSummaryPlot();
    summaryPlot->enableAutoPlotTitle( true );

    for ( const auto& addr : addresses )
    {
        for ( const auto ensemble : ensembles )
        {
            if ( !ensemble ) continue;

            auto curveSet = addNewEnsembleCurve( summaryPlot, addr, ensemble );
            curveSet->findOrAssignBottomAxisX( RiuPlotAxis::defaultBottomForSummaryVectors() );
        }

        for ( const auto summaryCase : summaryCases )
        {
            if ( !summaryCase ) continue;

            addNewSummaryCurve( summaryPlot, addr, summaryCase );
        }
    }

    summaryPlot->applyDefaultCurveAppearances();
    summaryPlot->loadDataAndUpdate();
    summaryPlot->zoomAll();

    return summaryPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void appendCurvesToPlot( RimSummaryPlot*                           summaryPlot,
                         const std::set<RifEclipseSummaryAddress>& addresses,
                         const std::vector<RimSummaryCase*>&       summaryCases,
                         const std::vector<RimSummaryEnsemble*>&   ensembles )
{
    for ( const auto& addr : addresses )
    {
        for ( const auto ensemble : ensembles )
        {
            auto curveSet = createCurveSet( ensemble, addr );
            summaryPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );
            curveSet->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
        }

        for ( const auto summaryCase : summaryCases )
        {
            auto curve = createCurve( summaryCase, addr );
            summaryPlot->addCurveNoUpdate( curve );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEnsembleCurveSet* addNewEnsembleCurve( RimSummaryPlot* summaryPlot, const RiaSummaryCurveAddress& curveAddress, RimSummaryEnsemble* ensemble )
{
    auto* curveSet = new RimEnsembleCurveSet();

    curveSet->setSummaryEnsemble( ensemble );
    curveSet->setCurveAddress( curveAddress );

    cvf::Color3f curveColor =
        RimSummaryCurveAppearanceCalculator::computeTintedCurveColorForAddress( curveSet->summaryAddressY(),
                                                                                static_cast<int>(
                                                                                    summaryPlot->ensembleCurveSetCollection()->curveSetCount() ) );

    auto adr = curveSet->summaryAddressY();
    if ( adr.isHistoryVector() ) curveColor = RiaPreferencesSummary::current()->historyCurveContrastColor();

    curveSet->setColor( curveColor );

    summaryPlot->ensembleCurveSetCollection()->addCurveSet( curveSet );

    curveSet->setLeftOrRightAxisY( RiuPlotAxis::defaultLeft() );
    curveSet->setBottomOrTopAxisX( RiuPlotAxis::defaultBottomForSummaryVectors() );

    summaryPlot->curvesChanged.send();

    return curveSet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCurve* addNewSummaryCurve( RimSummaryPlot* summaryPlot, const RiaSummaryCurveAddress& curveAddress, RimSummaryCase* summaryCase )
{
    auto curve = createCurve( summaryCase, curveAddress.summaryAddressY() );

    curve->setSummaryCaseX( summaryCase );
    curve->setSummaryAddressX( curveAddress.summaryAddressX() );
    if ( curveAddress.summaryAddressX().category() != SummaryCategory::SUMMARY_TIME )
    {
        curve->setAxisTypeX( RiaDefines::HorizontalAxisType::SUMMARY_VECTOR );
    }

    summaryPlot->addCurveNoUpdate( curve );

    if ( curveAddress.summaryAddressX().category() != SummaryCategory::SUMMARY_TIME )
    {
        summaryPlot->findOrAssignPlotAxisX( curve );
    }

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RimSummaryMultiPlot*> selectedSummaryMultiPlots()
{
    std::set<RimSummaryMultiPlot*> plots;

    auto selectedTreeViewItems = RiuDockWidgetTools::selectedItemsInTreeView( RiuDockWidgetTools::plotMainWindowPlotsTreeName() );
    for ( auto item : selectedTreeViewItems )
    {
        if ( auto object = dynamic_cast<caf::PdmObjectHandle*>( item ) )
        {
            if ( auto summaryMultiPlot = object->firstAncestorOrThisOfType<RimSummaryMultiPlot>() )
            {
                plots.insert( summaryMultiPlot );
            }
        }
    }

    return plots;
}

} // namespace RiaSummaryPlotTools
