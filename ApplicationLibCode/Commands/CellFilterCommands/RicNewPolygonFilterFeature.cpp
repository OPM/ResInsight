/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewPolygonFilterFeature.h"

#include "RiaApplication.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonInView.h"

#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimGridView.h"
#include "RimPolygonFilter.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPolygonFilterFeature, "RicNewPolygonFilterFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicNewPolygonFilterFeature::RicNewPolygonFilterFeature()
    : RicBasicPolygonFeature( true /*multiselect*/ )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFilterFeature::onActionTriggered( bool isChecked )
{
    RimPolygon* polygonDataSource = nullptr;
    QVariant    userData          = this->userData();
    if ( !userData.isNull() && userData.canConvert<void*>() )
    {
        polygonDataSource = static_cast<RimPolygon*>( userData.value<void*>() );
    }

    auto cellFilterCollection = caf::SelectionManager::instance()->selectedItemOfType<RimCellFilterCollection>();
    if ( !cellFilterCollection )
    {
        RimGridView* activeView = RiaApplication::instance()->activeMainOrComparisonGridView();
        if ( activeView )
        {
            cellFilterCollection = activeView->cellFilterCollection();
        }
    }
    if ( !cellFilterCollection ) return;

    auto sourceCase = cellFilterCollection->firstAncestorOrThisOfTypeAsserted<Rim3dView>()->ownerCase();
    if ( !sourceCase ) return;

    std::vector<RimPolygon*> polygons;

    if ( polygonDataSource )
    {
        polygons.push_back( polygonDataSource );
    }
    else
    {
        polygons = selectedPolygons();
    }

    RimPolygonFilter* lastItem = nullptr;

    if ( polygons.empty() )
    {
        lastItem = cellFilterCollection->addNewPolygonFilter( sourceCase, nullptr );
    }
    else
    {
        for ( auto polygon : polygons )
        {
            lastItem = cellFilterCollection->addNewPolygonFilter( sourceCase, polygon );
        }
    }

    if ( lastItem )
    {
        Riu3DMainWindowTools::selectAsCurrentItem( lastItem );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFilterFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Polygon.png" ) );
    actionToSetup->setText( "User Defined Polygon Filter" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewPolygonFilterFeature::isCommandEnabled() const
{
    return true;
}
