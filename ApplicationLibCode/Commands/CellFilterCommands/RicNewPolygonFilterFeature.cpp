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

#include "Polygons/RimPolygonInView.h"
#include "Polygons/RimPolygonTools.h"

#include "RimCase.h"
#include "RimCellFilterCollection.h"
#include "RimPolygonFilter.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafUtils.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewPolygonFilterFeature, "RicNewPolygonFilterFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFilter* RicNewPolygonFilterFeature::appendNewPolygonFilter( RimCase* sourceCase, RimCellFilterCollection* cellFilterCollection )
{
    if ( !sourceCase || !cellFilterCollection )
    {
        return nullptr;
    }

    RimPolygonFilter* polygonFilter = cellFilterCollection->addNewPolygonFilter( sourceCase );
    if ( polygonFilter )
    {
        auto polygon = RimPolygonTools::createNewPolygon();
        polygonFilter->setPolygon( polygon );

        RimPolygonTools::selectPolygonInView( polygon, polygonFilter );
        return polygonFilter;
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFilterFeature::onActionTriggered( bool isChecked )
{
    std::vector<RimCellFilterCollection*> colls = caf::selectedObjectsByTypeStrict<RimCellFilterCollection*>();
    if ( colls.empty() ) return;
    RimCellFilterCollection* filtColl = colls[0];

    RimCase* sourceCase = filtColl->firstAncestorOrThisOfTypeAsserted<RimCase>();

    appendNewPolygonFilter( sourceCase, filtColl );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewPolygonFilterFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/CellFilter_Polygon.png" ) );
    actionToSetup->setText( "New Polygon Filter" );
}
