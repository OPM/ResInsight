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

#include "RicHideGeometryForWellVisibilityFeature.h"

#include "RiaApplication.h"

#include "RimEclipseView.h"
#include "RimFaultInViewCollection.h"
#include "RimGridCollection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimSimWellInViewCollection.h"
#include "RimTools.h"
#include "WellPath/RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicHideGeometryForWellVisibilityFeature, "RicHideGeometryForWellVisibilityFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHideGeometryForWellVisibilityFeature::isCommandEnabled() const
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    return dynamic_cast<RimGridView*>( view ) != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHideGeometryForWellVisibilityFeature::onActionTriggered( bool isChecked )
{
    Rim3dView* view = RiaApplication::instance()->activeMainOrComparisonGridView();
    if ( !view ) return;

    RimGridView* gridView = dynamic_cast<RimGridView*>( view );
    if ( !gridView ) return;

    // Hide faults (Eclipse views only)
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( gridView );
    if ( eclipseView )
    {
        eclipseView->faultCollection()->setActive( false );
        eclipseView->faultCollection()->updateConnectedEditors();
    }

    // Hide intersections
    gridView->intersectionCollection()->setActive( false );
    gridView->intersectionCollection()->updateConnectedEditors();

    // Hide all reservoir geometry
    gridView->gridCollection()->setActive( false );
    gridView->gridCollection()->updateConnectedEditors();

    RiuMainWindow::instance()->refreshDrawStyleActions();
    RiuMainWindow::instance()->refreshAnimationActions();

    gridView->scheduleCreateDisplayModelAndRedraw();

    // Force simulation wells visible (Eclipse views only)
    if ( eclipseView )
    {
        RimSimWellInViewCollection* simWellColl = eclipseView->wellCollection();
        if ( simWellColl )
        {
            simWellColl->isActive = true;
            simWellColl->updateConnectedEditors();
        }
    }

    // Force all well paths visible
    RimWellPathCollection* wellPathColl = RimTools::wellPathCollection();
    if ( wellPathColl )
    {
        wellPathColl->isActive           = true;
        wellPathColl->wellPathVisibility = RimWellPathCollection::FORCE_ALL_ON;
        wellPathColl->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHideGeometryForWellVisibilityFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Hide All Geometry (Show Wells)" );
    actionToSetup->setIcon( QIcon( ":/Well.svg" ) );
}
