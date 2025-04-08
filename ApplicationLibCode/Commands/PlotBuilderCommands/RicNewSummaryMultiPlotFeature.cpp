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

#include "Summary/RiaSummaryPlotTools.h"

#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryEnsemble.h"
#include "RimSummaryMultiPlot.h"
#include "RimSummaryMultiPlotCollection.h"
#include "RimSummaryPlot.h"

#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryMultiPlotFeature, "RicNewSummaryMultiPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryMultiPlotFeature::isCommandEnabled() const
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();

    if ( selectedCollection( selectedItems ) ) return true;

    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;
    return selectedCases( &selectedIndividualSummaryCases, &selectedEnsembles );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::onActionTriggered( bool isChecked )
{
    const auto selectedItems = caf::SelectionManager::instance()->selectedItems();

    std::vector<RimSummaryCase*>     selectedIndividualSummaryCases;
    std::vector<RimSummaryEnsemble*> selectedEnsembles;

    RimSummaryMultiPlotCollection* coll = selectedCollection( selectedItems );
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
    RiaSummaryPlotTools::createAndAppendDefaultSummaryMultiPlot( selectedIndividualSummaryCases,
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
RimSummaryMultiPlotCollection* RicNewSummaryMultiPlotFeature::selectedCollection( const std::vector<caf::PdmUiItem*>& items )
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

    *selectedIndividualSummaryCases = caf::SelectionManager::instance()->objectsByTypeStrict<RimSummaryCase>();
    if ( !selectedEnsembles->empty() )
    {
        return true;
    }
    // Second try selected summary cases
    *selectedEnsembles = caf::SelectionManager::instance()->objectsByTypeStrict<RimSummaryEnsemble>();
    return !selectedIndividualSummaryCases->empty();
}
