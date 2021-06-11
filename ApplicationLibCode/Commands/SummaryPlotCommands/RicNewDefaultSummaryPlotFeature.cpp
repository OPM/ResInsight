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

#include "RiaSummaryTools.h"

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
#include "RimSummaryCaseCollection.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewDefaultSummaryPlotFeature, "RicNewDefaultSummaryPlotFeature" );

void extractPlotObjectsFromSelection( std::vector<RimSummaryCase*>*           selectedIndividualSummaryCases,
                                      std::vector<RimSummaryCaseCollection*>* selectedEnsembles )
{
    CAF_ASSERT( selectedIndividualSummaryCases && selectedEnsembles );

    // First try selected ensembles
    caf::SelectionManager::instance()->objectsByTypeStrict( selectedEnsembles );
    if ( !selectedEnsembles->empty() )
    {
        return;
    }
    // Second try selected summary cases
    caf::SelectionManager::instance()->objectsByTypeStrict( selectedIndividualSummaryCases );
    if ( !selectedIndividualSummaryCases->empty() )
    {
        return;
    }

    RimSummaryPlotCollection* sumPlotColl =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimSummaryPlotCollection>();

    if ( sumPlotColl )
    {
        RimSummaryCase*           firstIndividualSummaryCase = nullptr;
        RimSummaryCaseCollection* firstEnsemble              = nullptr;

        auto sumCaseVector = RimProject::current()->allSummaryCases();
        for ( RimSummaryCase* summaryCase : sumCaseVector )
        {
            RimSummaryCaseCollection* parentEnsemble = nullptr;
            summaryCase->firstAncestorOrThisOfType( parentEnsemble );
            if ( !parentEnsemble && !firstIndividualSummaryCase )
            {
                firstIndividualSummaryCase = summaryCase;
                break;
            }
            else if ( parentEnsemble && !firstEnsemble )
            {
                firstEnsemble = parentEnsemble;
            }
        }
        if ( firstIndividualSummaryCase )
        {
            selectedIndividualSummaryCases->push_back( firstIndividualSummaryCase );
        }
        else if ( firstEnsemble )
        {
            selectedEnsembles->push_back( firstEnsemble );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryPlot* RicNewDefaultSummaryPlotFeature::createFromSummaryCases( RimSummaryPlotCollection* plotCollection,
                                                                         const std::vector<RimSummaryCase*>& summaryCases )
{
    RimSummaryPlot* newPlot = plotCollection->createSummaryPlotWithAutoTitle();

    for ( RimSummaryCase* sumCase : summaryCases )
    {
        RicSummaryPlotFeatureImpl::addDefaultCurvesToPlot( newPlot, sumCase );
    }

    newPlot->applyDefaultCurveAppearances();
    newPlot->loadDataAndUpdate();

    plotCollection->updateConnectedEditors();

    RiuPlotMainWindowTools::setExpanded( newPlot );
    RiuPlotMainWindowTools::selectAsCurrentItem( newPlot );
    return newPlot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewDefaultSummaryPlotFeature::isCommandEnabled()
{
    std::vector<RimSummaryCase*>           selectedIndividualSummaryCases;
    std::vector<RimSummaryCaseCollection*> selectedEnsembles;

    extractPlotObjectsFromSelection( &selectedIndividualSummaryCases, &selectedEnsembles );

    RimCustomObjectiveFunctionCollection* customObjFuncCollection = nullptr;
    RimEnsembleCurveFilter*               curveFilter             = nullptr;
    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        selObj->firstAncestorOrThisOfType( customObjFuncCollection );
        selObj->firstAncestorOrThisOfType( curveFilter );
    }
    if ( customObjFuncCollection || curveFilter ) return false;

    return !( selectedIndividualSummaryCases.empty() && selectedEnsembles.empty() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewDefaultSummaryPlotFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimSummaryCase*>           selectedIndividualSummaryCases;
    std::vector<RimSummaryCaseCollection*> selectedEnsembles;
    extractPlotObjectsFromSelection( &selectedIndividualSummaryCases, &selectedEnsembles );

    if ( !selectedIndividualSummaryCases.empty() )
    {
        RimSummaryPlotCollection* sumPlotColl = RimProject::current()->mainPlotCollection()->summaryPlotCollection();
        createFromSummaryCases( sumPlotColl, selectedIndividualSummaryCases );
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
    std::vector<RimSummaryCase*>           selectedIndividualSummaryCases;
    std::vector<RimSummaryCaseCollection*> selectedEnsembles;

    extractPlotObjectsFromSelection( &selectedIndividualSummaryCases, &selectedEnsembles );

    if ( !selectedIndividualSummaryCases.empty() )
    {
        actionToSetup->setText( "New Summary Plot" );
    }
    else
    {
        actionToSetup->setText( "New Ensemble Summary Plot" );
    }
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
