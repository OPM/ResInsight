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

#include "RicNewSummaryPlotFromDataVectorFeature.h"

#include "RiaSummaryTools.h"

#include "RimSummaryAddress.h"

#include "cafSelectionManagerTools.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewSummaryPlotFromDataVectorFeature, "RicNewSummaryPlotFromDataVectorFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewSummaryPlotFromDataVectorFeature::isCommandEnabled()
{
    //std::vector<RimSummaryAddress*> selectedItems = caf::selectedObjectsByType<RimSummaryAddress*>();

    //for ( auto item : selectedItems )
    //{
    //    if ( !dynamic_cast<RimSummaryAddress*>( item ) && !dynamic_cast<RimSummaryCaseCollection*>( item ) )
    //        return false;
    //}
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFromDataVectorFeature::onActionTriggered( bool isChecked )
{
    // RimProject* project = RimProject::current();
    // CVF_ASSERT( project );

    // std::vector<RimSummaryCase*>           selectedCases  = caf::selectedObjectsByType<RimSummaryCase*>();
    // std::vector<RimSummaryCaseCollection*> selectedGroups = caf::selectedObjectsByType<RimSummaryCaseCollection*>();

    // std::vector<caf::PdmObject*> sourcesToSelect( selectedCases.begin(), selectedCases.end() );

    // if ( sourcesToSelect.empty() && selectedGroups.empty() )
    //{
    //    const auto allSingleCases = project->firstSummaryCaseMainCollection()->topLevelSummaryCases();
    //    const auto allGroups      = project->summaryGroups();
    //    std::vector<RimSummaryCaseCollection*> allEnsembles;
    //    for ( const auto group : allGroups )
    //        if ( group->isEnsemble() ) allEnsembles.push_back( group );

    //    if ( !allSingleCases.empty() )
    //    {
    //        sourcesToSelect.push_back( allSingleCases.front() );
    //    }
    //    else if ( !allEnsembles.empty() )
    //    {
    //        sourcesToSelect.push_back( allEnsembles.front() );
    //    }
    //}

    //// Append grouped cases
    // for ( auto group : selectedGroups )
    //{
    //    if ( group->isEnsemble() )
    //    {
    //        sourcesToSelect.push_back( group );
    //    }
    //    else
    //    {
    //        auto groupCases = group->allSummaryCases();
    //        sourcesToSelect.insert( sourcesToSelect.end(), groupCases.begin(), groupCases.end() );
    //    }
    //}

    // auto dialog = RicEditSummaryPlotFeature::curveCreatorDialog();

    // if ( !dialog->isVisible() )
    //{
    //    dialog->show();
    //}
    // else
    //{
    //    dialog->raise();
    //}

    // dialog->updateFromDefaultCases( sourcesToSelect );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewSummaryPlotFromDataVectorFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Summary Plot" );
    actionToSetup->setIcon( QIcon( ":/SummaryPlotLight16x16.png" ) );
}
