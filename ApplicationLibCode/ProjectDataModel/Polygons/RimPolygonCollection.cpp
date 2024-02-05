/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimPolygonCollection.h"
#include "RimPolygon.h"
#include "RimPolygonFile.h"

CAF_PDM_SOURCE_INIT( RimPolygonCollection, "RimPolygonCollection" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCollection::RimPolygonCollection()
{
    CAF_PDM_InitObject( "Polygons", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_polygons, "Polygons", "Polygons" );
    CAF_PDM_InitFieldNoDefault( &m_polygonFiles, "PolygonFiles", "Polygon Files" );

    auto polygon = new RimPolygon();
    polygon->setName( "Polygon 1" );
    m_polygons().push_back( polygon );

    auto polygonFile = new RimPolygonFile();
    polygonFile->setName( "Polygon 2" );
    polygonFile->loadData();
    m_polygonFiles().push_back( polygonFile );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonCollection::~RimPolygonCollection()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::loadData()
{
    for ( auto& p : m_polygonFiles() )
    {
        p->loadData();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::addPolygon( RimPolygon* polygon )
{
    m_polygons().push_back( polygon );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::deletePolygons()
{
    m_polygons().deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
}
