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
#include "RiaPreferences.h"

#include "RiaSummaryTools.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimEnsembleCurveSet.h"
#include "RimEnsembleCurveSetCollection.h"
#include "RimEnsembleCurveSetColorManager.h"
#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryEnsembleCurveSetFeature, "RicNewSummaryEnsembleCurveSetFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimEnsembleCurveSet*>
    RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( RimSummaryPlot* plot, RimSummaryCaseCollection* ensemble )
{
    CVF_ASSERT( plot && ensemble );

    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    QString     curvesTextFilter = RiaApplication::instance()->preferences()->defaultSummaryCurvesTextFilter;
    QStringList curveFilters     = curvesTextFilter.split( ";", QString::SkipEmptyParts );

    std::set<RifEclipseSummaryAddress> addrs = ensemble->ensembleSummaryAddresses();

    std::vector<RimEnsembleCurveSet*> curveSets;
    for ( const auto& addr : addrs )
    {
        for ( auto filter : curveFilters )
        {
            if ( addr.isUiTextMatchingFilterText( filter ) )
            {
                RimEnsembleCurveSet* curveSet = new RimEnsembleCurveSet();

                // Use same counting as RicNewSummaryCurveFeature::onActionTriggered
                auto colorIndex = plot->singleColorCurveCount();
                curveSet->setColor( RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( colorIndex ) );
                curveSet->legendConfig()->setColorLegend( RimRegularLegendConfig::mapToColorLegend(
                    RimEnsembleCurveSetColorManager::cycledEnsembleColorRange( static_cast<int>( colorIndex ) ) ) );

                curveSet->setSummaryCaseCollection( ensemble );
                curveSet->setSummaryAddress( addr );
                curveSet->filterCollection()->addFilter();

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
RimSummaryPlot*
    RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetsAndUpdate( std::vector<RimSummaryCaseCollection*> ensembles )
{
    RiaGuiApplication* app  = RiaGuiApplication::instance();
    RimProject*        proj = app->project();

    if ( RiaApplication::instance()->preferences()->defaultSummaryCurvesTextFilter().isEmpty() ) return nullptr;

    RimSummaryPlotCollection* summaryPlotCollection = proj->mainPlotCollection->summaryPlotCollection();
    RimSummaryPlot*           plot                  = summaryPlotCollection->createSummaryPlotWithAutoTitle();

    RimEnsembleCurveSet* firstCurveSetCreated = nullptr;
    for ( RimSummaryCaseCollection* ensemble : ensembles )
    {
        std::vector<RimEnsembleCurveSet*> curveSets =
            RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( plot, ensemble );
        if ( !firstCurveSetCreated && !curveSets.empty() ) firstCurveSetCreated = curveSets.front();
    }

    plot->loadDataAndUpdate();
    summaryPlotCollection->updateConnectedEditors();

    RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
    if ( mainPlotWindow )
    {
        mainPlotWindow->selectAsCurrentItem( firstCurveSetCreated );
        mainPlotWindow->updateSummaryPlotToolBar();
    }
    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryEnsembleCurveSetFeature::isCommandEnabled()
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

        RimEnsembleCurveSet* firstCurveSet = nullptr;
        if ( !RiaApplication::instance()->preferences()->defaultSummaryCurvesTextFilter().isEmpty() )
        {
            auto curveSets = RicNewSummaryEnsembleCurveSetFeature::addDefaultCurveSets( plot, ensemble );
            if ( !curveSets.empty() ) firstCurveSet = curveSets.front();
        }
        plot->loadDataAndUpdate();
        plot->updateConnectedEditors();

        RiaGuiApplication* app            = RiaGuiApplication::instance();
        RiuPlotMainWindow* mainPlotWindow = app->getOrCreateAndShowMainPlotWindow();
        if ( mainPlotWindow && firstCurveSet )
        {
            mainPlotWindow->selectAsCurrentItem( firstCurveSet );
            mainPlotWindow->updateSummaryPlotToolBar();
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
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot( selObj );
    }

    return sumPlot;
}
