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
void RimPolygonCollection::addUserDefinedPolygon( RimPolygon* polygon )
{
    m_polygons().push_back( polygon );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::deleteUserDefinedPolygons()
{
    m_polygons().deleteChildren();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::addPolygonFile( RimPolygonFile* polygonFile )
{
    m_polygonFiles().push_back( polygonFile );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCollection::userDefinedPolygons() const
{
    return m_polygons.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygonFile*> RimPolygonCollection::polygonFiles() const
{
    return m_polygonFiles.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonCollection::allPolygons() const
{
    std::vector<RimPolygon*> allPolygons;

    for ( auto& p : m_polygonFiles() )
    {
        for ( auto& polygon : p->polygons() )
        {
            allPolygons.push_back( polygon );
        }
    }

    for ( auto& polygon : m_polygons.childrenByType() )
    {
        allPolygons.push_back( polygon );
    }

    return allPolygons;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonCollection::onChildDeleted( caf::PdmChildArrayFieldHandle* childArray, std::vector<caf::PdmObjectHandle*>& referringObjects )
{
}
