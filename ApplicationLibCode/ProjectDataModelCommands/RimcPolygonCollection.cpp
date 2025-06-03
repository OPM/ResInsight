/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025- Equinor ASA
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

#include "RimcPolygonCollection.h"

#include "Polygons/RimPolygon.h"
#include "Polygons/RimPolygonCollection.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapabilityCvfVec3d.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimPolygonCollection, RimcPolygonCollection_createPolygon, "CreatePolygon" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcPolygonCollection_createPolygon::RimcPolygonCollection_createPolygon( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Create and Add New Polygon", "", "", "Create and Add New Polygon" );
    setNullptrValid( false );
    setResultPersistent( true );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinates, "Coordinates", "Coordinates" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcPolygonCollection_createPolygon::execute()
{
    auto polygonCollection = self<RimPolygonCollection>();

    QString                 name   = m_name();
    std::vector<cvf::Vec3d> coords = m_coordinates();

    RimPolygon* polygon = new RimPolygon();
    if ( !name.isEmpty() )
    {
        polygon->setName( name );
    }

    polygon->setPointsInDomainCoords( coords );

    polygonCollection->addUserDefinedPolygon( polygon );
    polygonCollection->updateConnectedEditors();

    return polygon;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcPolygonCollection_createPolygon::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimPolygon );
}
