/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RimPolygonTools.h"

#include "RimGridView.h"
#include "RimOilField.h"
#include "RimPolygon.h"
#include "RimPolygonCollection.h"
#include "RimPolygonInView.h"
#include "RimPolygonInViewCollection.h"
#include "RimProject.h"

#include "Riu3DMainWindowTools.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonTools::selectAndActivatePolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject )
{
    auto polygonInView = findPolygonInView( polygon, sourceObject );
    if ( polygonInView )
    {
        polygonInView->enablePicking( true );
        Riu3DMainWindowTools::selectAsCurrentItem( polygonInView );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInView* RimPolygonTools::findPolygonInView( RimPolygon* polygon, caf::PdmObject* sourceObject )
{
    if ( !polygon || !sourceObject )
    {
        return nullptr;
    }

    if ( auto gridView = sourceObject->firstAncestorOrThisOfType<RimGridView>() )
    {
        auto polyCollection = gridView->polygonInViewCollection();

        for ( auto polygonInView : polyCollection->polygonsInView() )
        {
            if ( polygonInView && polygonInView->polygon() == polygon )
            {
                return polygonInView;
            }
        }
    }

    return nullptr;
}
