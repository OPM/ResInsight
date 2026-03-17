/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RicShowAllGeometryFeature.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "WellPath/RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicShowAllGeometryFeature, "RicShowAllGeometryFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicShowAllGeometryFeature::isCommandEnabled() const
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    return dynamic_cast<RimGridView*>( view ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowAllGeometryFeature::onActionTriggered( bool isChecked )
{
    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return;

    RimGridView* gridView = dynamic_cast<RimGridView*>( view );
    if ( !gridView ) return;

    // Show faults (Eclipse views only)
    if ( auto eclipseView = dynamic_cast<RimEclipseView*>( gridView ) )
    {
        eclipseView->faultCollection()->setActive( true );
        eclipseView->faultCollection()->updateConnectedEditors();
    }

    // Show intersections
    gridView->intersectionCollection()->setActive( true );
    gridView->intersectionCollection()->updateConnectedEditors();

    // Show all reservoir geometry
    gridView->showGridCells( true );

    RiuMainWindow::instance()->refreshDrawStyleActions();
    RiuMainWindow::instance()->refreshAnimationActions();

    gridView->scheduleCreateDisplayModelAndRedraw();

    // Restore well path visibility to individual control
    if ( auto wellPathColl = RimWellPathCollection::instance() )
    {
        wellPathColl->wellPathVisibility = RimWellPathCollection::ALL_ON;
        wellPathColl->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicShowAllGeometryFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Show All Geometry" );
    actionToSetup->setIcon( QIcon( ":/draw_style_surface_24x24.png" ) );
}
