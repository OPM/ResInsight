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

#include "RimPolygonFile.h"
#include "RimPolygon.h"

CAF_PDM_SOURCE_INIT( RimPolygonFile, "RimPolygonFileFile" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPolygonFile::RimPolygonFile()
{
    CAF_PDM_InitObject( "PolygonFile", ":/PolylinesFromFile16x16.png" );

    CAF_PDM_InitFieldNoDefault( &m_fileName, "StimPlanFileName", "File Name" );
    CAF_PDM_InitFieldNoDefault( &m_polygons, "Polygons", "Polygons" );

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::loadData()
{
    loadPolygonsFromFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPolygon*> RimPolygonFile::polygons() const
{
    return m_polygons.childrenByType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    loadPolygonsFromFile();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimPolygonFile::loadPolygonsFromFile()
{
    // m_polygons()->deletePolygons();

    auto polygon = new RimPolygon();
    polygon->setName( "Polygon 1" );
    m_polygons.push_back( polygon );

    polygon = new RimPolygon();
    polygon->setName( "Polygon 2" );
    m_polygons.push_back( polygon );
}
