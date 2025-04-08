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

#include "RicNewDefaultSummaryPlotFeature.h"

#include "Summary/RiaSummaryPlotTools.h"
#include "Summary/RiaSummaryTools.h"

#include "RicEditSummaryPlotFeature.h"
#include "RicNewSummaryCurveFeature.h"
#include "RicNewSummaryEnsembleCurveSetFeature.h"
#include "RicSummaryPlotEditorDialog.h"
#include "RicSummaryPlotEditorUi.h"
#include "RicSummaryPlotFeatureImpl.h"

#include "RimCustomObjectiveFunctionCollection.h"
#include "RimEnsembleCurveFilter.h"
#include "RimEnsembleCurveFilterCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimRegularLegendConfig.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryPlot.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDefaultSummaryPlotFeature, "RicNewDefaultSummaryPlotFeature" );

void extractPlotObjectsFromSelection( std::vector<RimSummaryCase*>*     selectedIndividualSummaryCases,
                                      std::vector<RimSummaryEnsemble*>* selectedEnsembles )
{
    CAF_ASSERT( selectedIndividualSummaryCases && selectedEnsembles );

    // First try selected ensembles
    *selectedEnsembles = caf::SelectionManager::instance()->objectsByTypeStrict<RimSummaryEnsemble>();
    if ( !selectedEnsembles->empty() )
    {
        return;
    }
    // Second try selected summary cases
    *selectedIndividualSummaryCases = caf::SelectionManager::instance()->objectsByTypeStrict<RimSummaryCase>();
    if ( !selectedIndividualSummaryCases->empty() )
    {
        return;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewDefaultSummaryPlotFeature::createFromSummaryCases( const std::vector<RimSummaryCase*>& summaryCases )
{
    RimSummaryPlot* newPlot = new RimSummaryPlot();
    newPlot->enableAutoPlotTitle( true );

    for ( RimSummaryCase* sumCase : summaryCases )
    {
        RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot( newPlot, sumCase );
    }

    newPlot->applyDefaultCurveAppearances();
    newPlot->loadDataAndUpdate();

    RiaSummaryPlotTools::createAndAppendSingleSummaryMultiPlot( newPlot );

    return newPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDefaultSummaryPlotFeature::isCommandEnabled() const
{
    RimSummaryMultiPlot* multiPlot = dynamic_cast<RimSummaryMultiPlot*>( caf::SelectionManager::instance()->selectedItem() );
    if ( multiPlot )
    {
        return true;
    }

    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;

    extractPlotObjectsFromSelection( &selectedIndividualSummaryCases, &selectedEnsembles );

    RimCustomObjectiveFunctionCollection* customObjFuncCollection = nullptr;
    RimEnsembleCurveFilter*               curveFilter             = nullptr;
    caf::PdmObject*                       selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        customObjFuncCollection = selObj->firstAncestorOrThisOfType<RimCustomObjectiveFunctionCollection>();
        curveFilter             = selObj->firstAncestorOrThisOfType<RimEnsembleCurveFilter>();
    }
    if ( customObjFuncCollection || curveFilter ) return false;

    return !( selectedIndividualSummaryCases.empty() && selectedEnsembles.empty() );

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDefaultSummaryPlotFeature::onActionTriggered( bool isChecked )
{
    RimSummaryMultiPlot* multiPlot = dynamic_cast<RimSummaryMultiPlot*>( caf::SelectionManager::instance()->selectedItem() );
    if ( multiPlot )
    {
        RimSummaryPlot* plot = new RimSummaryPlot();
        plot->enableAutoPlotTitle( true );
        RiaSummaryPlotTools::appendPlotsToSummaryMultiPlot( multiPlot, { plot } );
        return;
    }

    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;
    extractPlotObjectsFromSelection( &selectedIndividualSummaryCases, &selectedEnsembles );

    if ( !selectedIndividualSummaryCases.empty() )
    {
        createFromSummaryCases( selectedIndividualSummaryCases );
    }
    else
    {
        CAF_ASSERT( !selectedEnsembles.empty() );
        RicNewSummaryEnsembleCurveSetFeature::createPlotForCurveSetsAndUpdate( selectedEnsembles );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDefaultSummaryPlotFeature::setupActionLook( QAction* actionToSetup )
{
    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;

    extractPlotObjectsFromSelection( &selectedIndividualSummaryCases, &selectedEnsembles );

    if ( !selectedEnsembles.empty() )
    {
        actionToSetup->setText( "Add Ensemble Summary Plot" );
    }
    else
    {
        actionToSetup->setText( "Add Summary Plot" );
    }
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
