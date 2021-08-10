/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2021-     Equinor ASA
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

#include "RicDeleteWellPathFeature.h"

#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicDeleteWellPathFeature, "RicDeleteWellPathFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDeleteWellPathFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType( &objects );

    return !objects.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType( &wellPaths );

    if ( !wellPaths.empty() )
    {
        auto wpc = RimTools::wellPathCollection();

        for ( auto w : wellPaths )
        {
            for ( auto wl : w->allWellPathLaterals() )
            {
                wpc->deleteWell( wl );
            }
        }

        wpc->rebuildWellPathNodes();
        wpc->scheduleRedrawAffectedViews();
        wpc->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDeleteWellPathFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Delete Well Path" );
    actionToSetup->setIcon( QIcon( ":/Erase.svg" ) );
}
