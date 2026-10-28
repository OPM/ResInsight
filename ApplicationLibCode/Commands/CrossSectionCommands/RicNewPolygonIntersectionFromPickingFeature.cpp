/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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

#include "RicNewPolygonIntersectionFromPickingFeature.h"

#include "RiaApplication.h"

#include "RimExtrudedCurveIntersection.h"
#include "RimGridView.h"
#include "RimIntersectionCollection.h"
#include "RimTools.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"
#include "Polygons/RimPolygonTools.h"

#include "Riu3dSelectionManager.h"
#include "RiuViewer.h"

#include "cafCmdFeatureManager.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPolygonIntersectionFromPickingFeature, "RicNewPolygonIntersectionFromPickingFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPolygonIntersectionFromPickingFeature::isCommandEnabled() const
{
    return intersectionCollection() != nullptr && RimTools::polygonCollection() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonIntersectionFromPickingFeature::onActionTriggered( bool isChecked )
{
    RimIntersectionCollection* collection = intersectionCollection();
    if ( !collection ) return;

    RimPolygonCollection* polygonCollection = RimTools::polygonCollection();
    if ( !polygonCollection ) return;

    RimPolygon* polygon = polygonCollection->createUserDefinedPolygon();
    polygon->setName( "Polyline" );
    polygon->setIsClosed( false );
    polygonCollection->addUserDefinedPolygon( polygon );
    polygonCollection->uiCapability()->updateAllRequiredEditors();

    auto intersection = new RimExtrudedCurveIntersection();
    intersection->configureForProjectPolyLine( polygon );
    collection->appendIntersectionAndUpdate( intersection, false );

    Riu3dSelectionManager::instance()->deleteAllItems();

    // Select the polygon in the view with picking enabled, so that clicks in the 3D view append points
    RimPolygonTools::activate3dEditOfPolygonInView( polygon, collection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonIntersectionFromPickingFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CrossSection16x16.png" ) );
    actionToSetup->setText( "Polygon Intersection" );
}

//--------------------------------------------------------------------------------------------------
/// Use the active view when triggered from the 3D view context menu, otherwise the selected
/// intersection collection. Fall back to the active view if no intersection collection is selected.
//--------------------------------------------------------------------------------------------------
RimIntersectionCollection* RicNewPolygonIntersectionFromPickingFeature::intersectionCollection()
{
    const bool isTriggeredFromViewer = dynamic_cast<RiuViewer*>( caf::CmdFeatureManager::instance()->currentContextMenuTargetWidget() );
    if ( !isTriggeredFromViewer )
    {
        if ( auto selectedCollection = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimIntersectionCollection>() )
        {
            return selectedCollection;
        }
    }

    if ( RimGridView* activeView = RiaApplication::instance()->activeMainOrComparisonGridView() )
    {
        return activeView->intersectionCollection();
    }

    return nullptr;
}
