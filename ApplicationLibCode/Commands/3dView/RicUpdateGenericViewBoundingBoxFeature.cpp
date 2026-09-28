/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RicUpdateGenericViewBoundingBoxFeature.h"

#include "RimGeneric3dView.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicUpdateGenericViewBoundingBoxFeature, "RicUpdateGenericViewBoundingBoxFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicUpdateGenericViewBoundingBoxFeature::isCommandEnabled() const
{
    return dynamic_cast<RimGeneric3dView*>( caf::SelectionManager::instance()->selectedItem() ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUpdateGenericViewBoundingBoxFeature::onActionTriggered( bool isChecked )
{
    if ( auto view = dynamic_cast<RimGeneric3dView*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        view->recomputeDomainBoundingBoxAndUpdateGridBox();
        view->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUpdateGenericViewBoundingBoxFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Update Bounding Box" );
    actionToSetup->setIcon( QIcon( ":/3DWindow.svg" ) );
}
