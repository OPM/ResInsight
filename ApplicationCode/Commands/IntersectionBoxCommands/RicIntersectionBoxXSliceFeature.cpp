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

#include "RicIntersectionBoxXSliceFeature.h"

#include "RicIntersectionFeatureImpl.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimGridView.h"
#include "RimIntersectionBox.h"
#include "RimIntersectionCollection.h"
#include "RiuViewerCommands.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"

#include "cafCmdExecCommandManager.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicIntersectionBoxXSliceFeature, "RicIntersectionBoxXSliceFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicIntersectionBoxXSliceFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicIntersectionBoxXSliceFeature::onActionTriggered( bool isChecked )
{
    RicIntersectionFeatureImpl::createIntersectionBoxSlize("X-slice (Intersection box)", RimIntersectionBox::PLANE_STATE_X);

   // RimGridView* activeView = RiaApplication::instance()->activeGridView();
   // RimGridView* activeMainOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();
   //
   // if ( activeMainOrComparisonView )
   // {
   //     RimIntersectionCollection* coll = activeMainOrComparisonView->crossSectionCollection();
   //     CVF_ASSERT( coll );
   //
   //     RimIntersectionBox* intersectionBox = new RimIntersectionBox();
   //     intersectionBox->name               = QString( "X-slice (Intersection box)" );
   //
   //     coll->appendIntersectionBoxAndUpdate( intersectionBox );
   //
   //     cvf::Vec3d domainCoord = activeView->viewer()->viewerCommands()->lastPickPositionInDomainCoords();
   //     intersectionBox->setToDefaultSizeSlice( RimIntersectionBox::PLANE_STATE_X, domainCoord );
   //
   //     coll->updateConnectedEditors();
   //     RiuMainWindow::instance()->selectAsCurrentItem( intersectionBox, false );
   //
   //     activeMainOrComparisonView->showGridCells(false);
   //     RiuMainWindow::instance()->refreshDrawStyleActions();
   //
   //     activeView->scheduleCreateDisplayModelAndRedraw();
   // }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicIntersectionBoxXSliceFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/IntersectionXPlane16x16.png" ) );
    actionToSetup->setText( "X-slice Intersection Box" );
}
