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

#include "RicDockIn3dViewFeature.h"

#include "RiaGuiApplication.h"

#include "RimGridView.h"
#include "RiuMainWindow.h"

#include "RiuViewer.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QSettings>

CAF_CMD_SOURCE_INIT( RicDockIn3dViewFeature, "RicDockIn3dViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDockIn3dViewFeature::isCommandEnabled() const
{
    if ( auto view = dynamic_cast<Rim3dView*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        if ( ( view->showWindow() ) && ( !view->isDockedIn3DView() ) )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDockIn3dViewFeature::onActionTriggered( bool isChecked )
{
    if ( auto view = dynamic_cast<Rim3dView*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        view->dockInMainWindow();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDockIn3dViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Dock in 3D Window" );
    actionToSetup->setIcon( QIcon( ":/3DWindow.svg" ) );
}
