/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicNewSummaryEnsembleCurveSetFeature.h"

#include "RiaColorTables.h"
#include "RiaGuiApplication.h"
#include "RiaPreferencesSummary.h"
#include "RiaSummaryTools.h"
#include "RiaTextStringTools.h"

#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveAppearanceCalculator.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"

#include "PlotBuilderCommands/RicSummaryPlotBuilder.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryEnsembleCurveSetFeature, "RicNewSummaryEnsembleCurveSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*> RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( RimSummaryPlot* plot, RimSummaryEnsemble* ensemble )
{
    CVF_ASSERT( plot && ensemble );

    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    QString     curvesTextFilter = prefs->defaultSummaryCurvesTextFilter();
    QStringList curveFilters     = RiaTextStringTools::splitSkipEmptyParts( curvesTextFilter, ";" );

    std::set<RifEclipseSummaryAddress> addrs = ensemble->ensembleSummaryAddresses();

    std::vector<RimEnsembleCurveSet*> curveSets;
    for ( const auto& addr : addrs )
    {
        for ( const auto& filter : curveFilters )
        {
            if ( addr.isUiTextMatchingFilterText( filter ) )
            {
                RimEnsembleCurveSet* curveSet = new RimEnsembleCurveSet();

                // Use same counting as RicNewSummaryCurveFeature::onActionTriggered
                auto colorIndex = plot->singleColorCurveCount();

                curveSet->setColor(
                    RimSummaryCurveAppearanceCalculator::computeTintedCurveColorForAddress( addr, static_cast<int>( colorIndex ) ) );
                curveSet->legendConfig()->setColorLegend( RimRegularLegendConfig::mapToColorLegend(
                    RimEnsembleCurveSetColorManager::cycledEnsembleColorRange( static_cast<int>( colorIndex ) ) ) );

                curveSet->setSummaryEnsemble( ensemble );
                curveSet->setSummaryAddressYAndStatisticsFlag( addr );
                auto filter = curveSet->filterCollection()->addFilter();
                filter->setActive( false );

                plot->ensembleCurveSetCollection()->addCurveSet( curveSet );
                curveSets.push_back( curveSet );
            }
        }
    }

    return curveSets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetsAndUpdate( std::vector<RimSummaryEnsemble*> ensembles )
{
    RiaGuiApplication* app = RiaGuiApplication::instance();

    RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

    if ( prefs->defaultSummaryCurvesTextFilter().isEmpty() ) return nullptr;

    RimSummaryPlot* plot = new RimSummaryPlot();
    plot->enableAutoPlotTitle( true );
    RimSummaryMultiPlot* multiPlot = RicSummaryPlotBuilder::createAndAppendSingleSummaryMultiPlot( plot );

    RimEnsembleCurveSet* firstCurveSetCreated = nullptr;
    for ( RimSummaryEnsemble* ensemble : ensembles )
    {
        std::vector<RimEnsembleCurveSet*> curveSets = RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( plot, ensemble );
        if ( !firstCurveSetCreated && !curveSets.empty() ) firstCurveSetCreated = curveSets.front();
    }

    plot->loadDataAndUpdate();
    multiPlot->updateConnectedEditors();

    RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if ( mainPlotWindow )
    {
        mainPlotWindow->selectAsCurrentItem( firstCurveSetCreated );
        mainPlotWindow->updateMultiPlotToolBar();
    }
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryEnsembleCurveSetFeature::isCommandEnabled() const
{
    bool summaryPlotSelected = selectedSummaryPlot();
    if ( summaryPlotSelected )
    {
        RimProject* project = RimProject::current();
        CVF_ASSERT( project );
        if ( !project->summaryGroups().empty() )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryEnsembleCurveSetFeature::onActionTriggered( bool isChecked )
{
    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimSummaryPlot* plot = selectedSummaryPlot();
    if ( plot )
    {
        CVF_ASSERT( !project->summaryGroups().empty() );
        auto ensemble = project->summaryGroups().back();

        RiaPreferencesSummary* prefs = RiaPreferencesSummary::current();

        RimEnsembleCurveSet* firstCurveSet = nullptr;
        if ( !prefs->defaultSummaryCurvesTextFilter().isEmpty() )
        {
            auto curveSets = RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( plot, ensemble );
            if ( !curveSets.empty() ) firstCurveSet = curveSets.front();
        }
        plot->loadDataAndUpdate();
        plot->updateAllRequiredEditors();

        RiaGuiApplication* app            = RiaGuiApplication::instance();
        RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
        if ( mainPlotWindow && firstCurveSet )
        {
            mainPlotWindow->selectAsCurrentItem( firstCurveSet );
            mainPlotWindow->updateMultiPlotToolBar();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryEnsembleCurveSetFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Ensemble Curve Set" );
    actionToSetup->setIcon( QIcon( ":/EnsembleCurveSet16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryEnsembleCurveSetFeature::selectedSummaryPlot() const
{
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        return RiaSummaryTools::parentSummaryPlot( selObj );
    }

    return nullptr;
}
