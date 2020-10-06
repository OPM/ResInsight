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

#include "RicViewZoomAllFeature.h"

#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimViewWindow.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"

#include <QAction>
#include <QMdiSubWindow>

CAF_CMD_SOURCE_INIT( RicViewZoomAllFeature, "RicViewZoomAllFeature" );

RimViewWindow* activeViewWindow()
{
    QWidget* topLevelWidget = RiaGuiApplication::activeWindow();

    if ( dynamic_cast<RiuMainWindow*>( topLevelWidget ) )
    {
        return RiaGuiApplication::instance()->activeReservoirView();
    }
    else if ( dynamic_cast<RiuPlotMainWindow*>( topLevelWidget ) )
    {
        RiuPlotMainWindow*    mainPlotWindow = dynamic_cast<RiuPlotMainWindow*>( topLevelWidget );
        QList<QMdiSubWindow*> subwindows     = mainPlotWindow->subWindowList( QMdiArea::StackingOrder );
        if ( !subwindows.empty() )
        {
            return RiuInterfaceToViewWindow::viewWindowFromWidget( subwindows.back()->widget() );
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicViewZoomAllFeature::isCommandEnabled()
{
    return activeViewWindow() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicViewZoomAllFeature::onActionTriggered( bool isChecked )
{
    this->disableModelChangeContribution();

    RimViewWindow* viewWindow = activeViewWindow();
    if ( viewWindow )
    {
        viewWindow->zoomAll();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicViewZoomAllFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Zoom All" );
    actionToSetup->setToolTip( "Zoom All (Ctrl+Alt+A)" );
    actionToSetup->setIcon( QIcon( ":/ZoomAll.svg" ) );
    applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+Alt+A" ) ) );
}
