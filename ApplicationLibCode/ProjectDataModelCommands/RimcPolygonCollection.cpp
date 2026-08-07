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

#include "RiaNameUniquenessTools.h"

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
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create and Add New Polygon", "", "", "Create and Add New Polygon" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_coordinates, "Coordinates", "Coordinates" );
    CAF_PDM_InitScriptableField( &m_onNameConflict,
                                 "OnNameConflict",
                                 caf::AppEnum<RiaDefines::NameConflictPolicy>( RiaDefines::NameConflictPolicy::FAIL ),
                                 "",
                                 "",
                                 "",
                                 "How to handle a polygon name already used in this folder" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcPolygonCollection_createPolygon::execute()
{
    auto polygonCollection = self<RimPolygonCollection>();

    QString                 name   = m_name();
    std::vector<cvf::Vec3d> coords = m_coordinates();

    if ( !name.isEmpty() )
    {
        auto resolution = RiaNameUniquenessTools::applyConflictPolicy( &polygonCollection->itemsField(), name, m_onNameConflict().value() );
        if ( !resolution.errorMessage.isEmpty() ) return std::unexpected( resolution.errorMessage );

        // The new polygon is added below, and that refreshes the views for both objects
        if ( auto* existingPolygon = dynamic_cast<RimPolygon*>( resolution.objectToReplace ) )
        {
            polygonCollection->deleteItem( existingPolygon );
        }

        name = resolution.nameToUse;
    }

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
QString RimcPolygonCollection_createPolygon::classKeywordReturnedType() const
{
    return RimPolygon::classKeywordStatic();
}
