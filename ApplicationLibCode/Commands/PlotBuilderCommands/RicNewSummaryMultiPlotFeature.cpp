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

#include "RimSummaryCase.h"
#include "RimSummaryCaseCollection.h"
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
bool RicNewSummaryMultiPlotFeature::isCommandEnabled()
{
    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    if ( selectedCollection( selectedUiItems ) ) return true;

    std::vector<RimSummaryCase*>           selectedIndividualSummaryCases;
    std::vector<RimSummaryCaseCollection*> selectedEnsembles;
    if ( selectedCases( &selectedIndividualSummaryCases, &selectedEnsembles ) ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryMultiPlotFeature::onActionTriggered( bool isChecked )
{
    std::vector<caf::PdmUiItem*> selectedUiItems;
    caf::SelectionManager::instance()->selectedItems( selectedUiItems );

    RimSummaryMultiPlotCollection* coll = selectedCollection( selectedUiItems );
    if ( coll )
    {
        std::vector<caf::PdmObjectHandle*> objects = {};
        RimSummaryMultiPlot* multiPlot             = RicSummaryPlotBuilder::createAndAppendSummaryMultiPlot( objects );

        return;
    }

    std::vector<RimSummaryCase*>           selectedIndividualSummaryCases;
    std::vector<RimSummaryCaseCollection*> selectedEnsembles;

    if ( selectedCases( &selectedIndividualSummaryCases, &selectedEnsembles ) )
    {
        RicSummaryPlotBuilder::createAndAppendDefaultSummaryMultiPlot( selectedIndividualSummaryCases, selectedEnsembles );
    }
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
bool RicNewSummaryMultiPlotFeature::selectedCases( std::vector<RimSummaryCase*>* selectedIndividualSummaryCases,
                                                   std::vector<RimSummaryCaseCollection*>* selectedEnsembles )
{
    CAF_ASSERT( selectedIndividualSummaryCases && selectedEnsembles );

    caf::SelectionManager::instance()->objectsByTypeStrict( selectedEnsembles );
    if ( !selectedEnsembles->empty() )
    {
        return true;
    }
    // Second try selected summary cases
    caf::SelectionManager::instance()->objectsByTypeStrict( selectedIndividualSummaryCases );
    if ( !selectedIndividualSummaryCases->empty() )
    {
        return true;
    }

    return false;
}
