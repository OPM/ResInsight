/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
#include "RicIntersectionFeatureImpl.h"

#include "RiaApplication.h"

#include "RimBoxIntersection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"

#include "RiuMainWindow.h"
#include "RiuViewer.h"
#include "RiuViewerCommands.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicIntersectionFeatureImpl::createIntersectionBoxSlize( const QString& name, RimBoxIntersection::SinglePlaneState plane )
{
    RimGridView* activeView                 = RiaApplication::instance()->activeGridView();
    RimGridView* activeMainOrComparisonView = RiaApplication::instance()->activeMainOrComparisonGridView();

    if ( activeMainOrComparisonView )
    {
        RimIntersectionCollection* coll = activeMainOrComparisonView->intersectionCollection();
        CVF_ASSERT( coll );

        cvf::Vec3d domainCoord = activeView->viewer()->viewerCommands()->lastPickPositionInDomainCoords();

        RimBoxIntersection* intersectionBox = new RimBoxIntersection();
        intersectionBox->setName( name );

        coll->appendIntersectionBoxNoUpdate( intersectionBox );
        intersectionBox->setToDefaultSizeSlice( plane, domainCoord );
        coll->updateConnectedEditors();

        activeMainOrComparisonView->showGridCells( false );
        activeMainOrComparisonView->scheduleCreateDisplayModelAndRedraw();
        activeView->scheduleCreateDisplayModelAndRedraw();

        RiuMainWindow::instance()->selectAsCurrentItem( intersectionBox, false );
        RiuMainWindow::instance()->refreshDrawStyleActions();
    }
}
