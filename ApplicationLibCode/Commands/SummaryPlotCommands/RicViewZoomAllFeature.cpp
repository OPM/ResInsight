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

CAF_CMD_SOURCE_INIT( RicViewZoomAllFeature, "RicViewZoomAllFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicViewZoomAllFeature::onActionTriggered( bool isChecked )
{
    disableModelChangeContribution();

    QWidget* topLevelWidget = RiaGuiApplication::activeWindow();

    if ( dynamic_cast<RiuMainWindow*>( topLevelWidget ) )
    {
        if ( auto viewWindow = RiaGuiApplication::instance()->activeReservoirView() )
        {
            viewWindow->zoomAll();
        }
    }
    else if ( auto plotMainWin = dynamic_cast<RiuPlotMainWindow*>( topLevelWidget ) )
    {
        if ( auto activePlotView = plotMainWin->activeViewer() )
        {
            activePlotView->zoomAllAndReleaseUserDefinedRanges();
        }
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
