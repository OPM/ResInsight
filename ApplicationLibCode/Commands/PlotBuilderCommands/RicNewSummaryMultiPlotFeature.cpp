/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicNewSummaryMultiPlotFeature.h"

#include "RicSummaryPlotBuilder.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "RicSummaryPlotBuilder.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryMultiPlotFeature, "RicNewSummaryMultiPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryMultiPlotFeature::isCommandEnabled() const
{
    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    if ( selectedCollection( selectedUiItems ) ) return true;

    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;
    return selectedCases( &selectedIndividualSummaryCases, &selectedEnsembles );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;

    RimSummaryMultiPlotCollection* coll = selectedCollection( selectedUiItems );
    if ( coll )
    {
        auto ensembles = RimProject::current()->summaryGroups();
        if ( !ensembles.empty() )
            selectedEnsembles.push_back( ensembles.front() );
        else
        {
            auto summaryCases = RimProject::current()->allSummaryCases();
            if ( !summaryCases.empty() ) selectedIndividualSummaryCases.push_back( summaryCases.front() );
        }
    }
    else
    {
        selectedCases( &selectedIndividualSummaryCases, &selectedEnsembles );
    }

    bool skipCreationOfPlotBasedOnPreferences = false;
    RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( selectedIndividualSummaryCases,
                                                                   selectedEnsembles,
                                                                   skipCreationOfPlotBasedOnPreferences );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Plot" );
    actionToSetup->setIcon( QIcon( ":/MultiPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryMultiPlotCollection* RicNewSummaryMultiPlotFeature::selectedCollection( std::vector<caf::PdmUiItem*>& items )
{
    for ( caf::PdmUiItem* uiItem : items )
    {
        RimSummaryMultiPlotCollection* coll = dynamic_cast<RimSummaryMultiPlotCollection*>( uiItem );
        if ( coll ) return coll;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryMultiPlotFeature::selectedCases( std::vector<RimSummaryCase*>*     selectedIndividualSummaryCases,
                                                   std::vector<RimSummaryEnsemble*>* selectedEnsembles )
{
    CAF_ASSERT( selectedIndividualSummaryCases && selectedEnsembles );

    caf::SelectionManager::instance()->objectsByTypeStrict( selectedEnsembles );
    if ( !selectedEnsembles->empty() )
    {
        return true;
    }
    // Second try selected summary cases
    caf::SelectionManager::instance()->objectsByTypeStrict( selectedIndividualSummaryCases );
    return !selectedIndividualSummaryCases->empty();
}
