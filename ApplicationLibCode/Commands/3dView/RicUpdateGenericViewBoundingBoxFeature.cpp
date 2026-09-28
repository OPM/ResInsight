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

#include "RiaApplication.h"

#include "RimGeneric3dView.h"

#include "RiuViewer.h"

#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicUpdateGenericViewBoundingBoxFeature, "RicUpdateGenericViewBoundingBoxFeature" );

namespace
{
//--------------------------------------------------------------------------------------------------
/// Finds the RimGeneric3dView to operate on: either the active view, when triggered from a 3D viewer context
/// menu, or the selected item, when triggered from the project tree.
//--------------------------------------------------------------------------------------------------
RimGeneric3dView* targetView()
{
    if ( dynamic_cast<RiuViewer*>( caf::CmdFeatureManager::instance()->currentContextMenuTargetWidget() ) )
    {
        return dynamic_cast<RimGeneric3dView*>( RiaApplication::instance()->activeReservoirView() );
    }

    return dynamic_cast<RimGeneric3dView*>( caf::SelectionManager::instance()->selectedItem() );
}
} // namespace

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicUpdateGenericViewBoundingBoxFeature::isCommandEnabled() const
{
    return targetView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicUpdateGenericViewBoundingBoxFeature::onActionTriggered( bool isChecked )
{
    if ( auto view = targetView() )
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
