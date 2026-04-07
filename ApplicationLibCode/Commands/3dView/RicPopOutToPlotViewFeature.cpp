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

#include "RicPopOutToPlotViewFeature.h"

#include "RiaGuiApplication.h"

#include "RimGridView.h"
#include "RiuMainWindow.h"

#include "RiuViewer.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QSettings>

CAF_CMD_SOURCE_INIT( RicPopOutToPlotViewFeature, "RicPopOutToPlotViewFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicPopOutToPlotViewFeature::isCommandEnabled() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPopOutToPlotViewFeature::onActionTriggered( bool isChecked )
{
    if ( auto view = dynamic_cast<Rim3dView*>( caf::SelectionManager::instance()->selectedItem() ) )
    {
        view->convertToDocking( RiaGuiApplication::instance()->mainPlotWindow() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicPopOutToPlotViewFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Pop Out (in Plot View)" );
    actionToSetup->setIcon( QIcon( ":/PlotWindow.svg" ) );
}
