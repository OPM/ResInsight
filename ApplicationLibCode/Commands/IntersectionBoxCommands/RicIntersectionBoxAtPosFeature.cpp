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

#include "RicIntersectionBoxAtPosFeature.h"

#include "RiaApplication.h"

#include "RimBoxIntersection.h"
#include "RimCase.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuViewerCommands.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicIntersectionBoxAtPosFeature, "RicIntersectionBoxAtPosFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicIntersectionBoxAtPosFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicIntersectionBoxAtPosFeature::onActionTriggered( bool isChecked )
{
    RimGridView* activeView                 = RiaApplication::instance()->activeGridView();
    RimGridView* activeMainOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( activeMainOrComparisonView )
    {
        RimIntersectionCollection* coll = activeMainOrComparisonView->intersectionCollection();
        CVF_ASSERT( coll );

        RimBoxIntersection* intersectionBox = new RimBoxIntersection();
        intersectionBox->setName( "Intersection box" );

        coll->appendIntersectionBoxAndUpdate( intersectionBox );

        cvf::Vec3d domainCoord = activeView->viewer()->viewerCommands()->lastPickPositionInDomainCoords();

        intersectionBox->setToDefaultSizeSlice( RimBoxIntersection::PLANE_STATE_NONE, domainCoord );

        coll->updateConnectedEditors();
        RiuMainWindow::instance()->selectAsCurrentItem( intersectionBox, false );

        activeMainOrComparisonView->showGridCells( false );
        RiuMainWindow::instance()->refreshDrawStyleActions();

        activeView->scheduleCreateDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicIntersectionBoxAtPosFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/IntersectionBox16x16.png" ) );
    actionToSetup->setText( "Intersection Box" );
}
