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

#include "RicToggle3DSelectionLinkFeature.h"

#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimViewWindow.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QMdiSubWindow>

CAF_CMD_SOURCE_INIT( RicToggle3DSelectionLinkFeature, "RicToggle3DSelectionLinkFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggle3DSelectionLinkFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggle3DSelectionLinkFeature::onActionTriggered( bool isChecked )
{
    // isChecked parameter isn't what we want, need to check that action check status
    isChecked = this->action()->isChecked();

    this->disableModelChangeContribution();

    QWidget*           topLevelWidget = RiaGuiApplication::activeWindow();
    RiuPlotMainWindow* plotWnd        = dynamic_cast<RiuPlotMainWindow*>( topLevelWidget );
    if ( plotWnd )
    {
        plotWnd->enable3DSelectionLink( isChecked );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggle3DSelectionLinkFeature::setupActionLook( QAction* actionToSetup )
{
    // bool isChecked = false;

    // QWidget*           topLevelWidget = RiaGuiApplication::activeWindow();
    // RiuPlotMainWindow* plotWnd        = dynamic_cast<RiuPlotMainWindow*>( topLevelWidget );
    // if ( plotWnd )
    //{
    //    isChecked = plotWnd->selection3DLinkEnabled();
    //}

    // actionToSetup->setChecked( isChecked );
    actionToSetup->setCheckable( true );
    actionToSetup->setText( "Link with selection in 3D view" );
    actionToSetup->setToolTip( "Update data sources in plots from well selections in 3D view." );
    actionToSetup->setIcon( QIcon( ":/Link3DandPlots.png" ) );
}
