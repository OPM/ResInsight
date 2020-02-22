/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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

#include "RicCollapseSiblingsFeature.h"

#include "Rim3dView.h"
#include "RimSummaryPlot.h"

#include "RiuMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicCollapseSiblingsFeature, "RicCollapseSiblingsFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicCollapseSiblingsFeature::isCommandEnabled()
{
    {
        auto selectedItem = dynamic_cast<Rim3dView*>( caf::SelectionManager::instance()->selectedItem() );
        if ( selectedItem ) return true;
    }

    {
        auto selectedItem = dynamic_cast<RimSummaryPlot*>( caf::SelectionManager::instance()->selectedItem() );
        if ( selectedItem ) return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCollapseSiblingsFeature::onActionTriggered( bool isChecked )
{
    auto selectedItem = caf::SelectionManager::instance()->selectedItem();

    RiuMainWindowTools::collapseSiblings( selectedItem );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicCollapseSiblingsFeature::setupActionLook( QAction* actionToSetup )
{
    QString objectName = "Items";

    {
        auto selectedItem = dynamic_cast<Rim3dView*>( caf::SelectionManager::instance()->selectedItem() );
        if ( selectedItem )
        {
            objectName = "Views";
        }
    }

    {
        auto selectedItem = dynamic_cast<RimSummaryPlot*>( caf::SelectionManager::instance()->selectedItem() );
        if ( selectedItem )
        {
            objectName = "Plots";
        }
    }

    actionToSetup->setText( "Collapse Other " + objectName );
    //    actionToSetup->setIcon(QIcon(":/ToggleOn16x16.png"));
}
