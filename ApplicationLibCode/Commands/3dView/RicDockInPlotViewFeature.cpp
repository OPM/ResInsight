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

#include "RicDockInPlotViewFeature.h"

#include "Rim3dView.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QSettings>

CAF_CMD_SOURCE_INIT( RicDockInPlotViewFeature, "RicDockInPlotViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicDockInPlotViewFeature::isCommandEnabled() const
{
    if ( auto view = dynamic_cast<Rim3dView*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        if ( ( view->showWindow() ) && ( !view->isDockedInPlotView() ) )
        {
            return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDockInPlotViewFeature::onActionTriggered( bool isChecked )
{
    if ( auto view = dynamic_cast<Rim3dView*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        view->dockInPlotWindow();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicDockInPlotViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Dock in Plot Window" );
    actionToSetup->setIcon( QIcon( ":/PlotWindow.svg" ) );
}
