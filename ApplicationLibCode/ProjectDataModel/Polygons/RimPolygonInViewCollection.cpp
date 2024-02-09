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

#include "RimPolygonInViewCollection.h"

#include "RimPolygon.h"
#include "RimPolygonCollection.h"
#include "RimPolygonInView.h"
#include "RimTools.h"

#include "cafDisplayCoordTransform.h"

#include "cvfModelBasicList.h"

CAF_PDM_SOURCE_INIT( RimPolygonInViewCollection, "RimPolygonInViewCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonInViewCollection::RimPolygonInViewCollection()
{
    CAF_PDM_InitObject( "Polygons", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_polygons, "Polygons", "Polygons" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::syncPolygonsInView()
{
    std::vector<RimPolygonInView*> existingPolygonsInView = m_polygons.childrenByType();
    m_polygons.clearWithoutDelete();

    auto polygonCollection = RimTools::polygonCollection();
    if ( polygonCollection )
    {
        std::vector<RimPolygonInView*> newPolygonsInView;

        for ( auto polygon : polygonCollection->allPolygons() )
        {
            auto it = std::find_if( existingPolygonsInView.begin(),
                                    existingPolygonsInView.end(),
                                    [polygon]( auto* polygonInView ) { return polygonInView->polygon() == polygon; } );

            if ( it != existingPolygonsInView.end() )
            {
                newPolygonsInView.push_back( *it );
                existingPolygonsInView.erase( it );
            }
            else
            {
                auto polygonInView = new RimPolygonInView();
                polygonInView->setPolygon( polygon );
                newPolygonsInView.push_back( polygonInView );
            }
        }

        m_polygons.setValue( newPolygonsInView );
    }

    for ( auto polyInView : existingPolygonsInView )
    {
        delete polyInView;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonInViewCollection::appendPartsToModel( cvf::ModelBasicList*        model,
                                                     caf::DisplayCoordTransform* scaleTransform,
                                                     const cvf::BoundingBox&     boundingBox )
{
    for ( auto polygon : m_polygons )
    {
        if ( polygon && polygon->isChecked() )
        {
            polygon->appendPartsToModel( model, scaleTransform, boundingBox );
        }
    }
}
