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

#include "RicNewSummaryCurveFeature.h"

#include "RiaColorTables.h"
#include "RiaGuiApplication.h"

#include "RiaSummaryTools.h"
#include "RimMainPlotCollection.h"
#include "RimObservedDataCollection.h"
#include "RimObservedSummaryData.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurve.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"

#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "RicSummaryPlotFeatureImpl.h"
#include "RiuPlotMainWindowTools.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryCurveFeature, "RicNewSummaryCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryCurveFeature::isCommandEnabled()
{
    return ( selectedSummaryPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app     = RiaGuiApplication::instance();
    RimProject*        project = app->project();
    CVF_ASSERT( project );

    RimSummaryPlot* plot = selectedSummaryPlot();
    if ( plot )
    {
        RimSummaryCase* defaultCase = nullptr;
        if ( !plot->summaryCurves().empty() )
        {
            defaultCase = plot->summaryCurves().back()->summaryCaseY();
        }

        if ( !defaultCase )
        {
            std::vector<RimSummaryCase*> allSummaryCases =
                project->activeOilField()->summaryCaseMainCollection()->allSummaryCases();

            if ( !allSummaryCases.empty() )
            {
                defaultCase = allSummaryCases.front();
            }
        }

        if ( !defaultCase )
        {
            auto allSummaryCases = project->activeOilField()->observedDataCollection()->allObservedSummaryData();

            if ( !allSummaryCases.empty() )
            {
                defaultCase = allSummaryCases.front();
            }
        }

        RimSummaryCurve* newCurve = new RimSummaryCurve();

        // Use same counting as RicNewSummaryEnsembleCurveSetFeature::onActionTriggered
        cvf::Color3f curveColor =
            RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( plot->singleColorCurveCount() );
        newCurve->setColor( curveColor );

        plot->addCurveNoUpdate( newCurve );
        newCurve->setSummaryCaseY( defaultCase );

        plot->loadDataAndUpdate();
        plot->updateConnectedEditors();

        app->getOrCreateAndShowMainPlotWindow()->selectAsCurrentItem( newCurve );

        RiuPlotMainWindow* mainPlotWindow = app->mainPlotWindow();
        mainPlotWindow->updateSummaryPlotToolBar();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Curve" );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewSummaryCurveFeature::selectedSummaryPlot() const
{
    RimSummaryPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        sumPlot = RiaSummaryTools::parentSummaryPlot( selObj );
    }

    return sumPlot;
}
