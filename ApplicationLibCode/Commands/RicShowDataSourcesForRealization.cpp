////////////////////////////////////////////////////////////////////////////////
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

#include "RicShowDataSourcesForRealization.h"

#include "RimSummaryCase.h"

#include "cafSelectionManager.h"
#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowDataSourcesForRealization, "RicShowDataSourcesForRealization" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowDataSourcesForRealization::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Show Data Sources" );

    actionToSetup->setCheckable( true );
    actionToSetup->setChecked( isCommandChecked() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowDataSourcesForRealization::isCommandChecked() const
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
    if ( !selection.empty() )
    {
        return selection.front()->showVectorItemsInProjectTree();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowDataSourcesForRealization::isCommandEnabled() const
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();

    for ( RimSummaryCase* summaryCase : selection )
    {
        if ( summaryCase->ensemble() )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowDataSourcesForRealization::onActionTriggered( bool isChecked )
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimSummaryCase>();
    if ( selection.empty() ) return;

    bool enableDataSources = !selection.front()->showVectorItemsInProjectTree();

    for ( auto summaryCase : selection )
    {
        summaryCase->setShowVectorItemsInProjectTree( enableDataSources );
    }
}
