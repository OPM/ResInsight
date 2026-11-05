/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RimcTriangulatedSurface.h"

#include "KeyValueStore/RiaKeyValueStoreUtil.h"
#include "RiaApplication.h"

#include "RimSurfaceCollection.h"
#include "RimTriangulatedSurface.h"
#include "RimcDataContainerString.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

#include <cmath>

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimTriangulatedSurface, RimcTriangulatedSurface_setGeometryFromKeys, "SetGeometryFromKeys" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcTriangulatedSurface_setGeometryFromKeys::RimcTriangulatedSurface_setGeometryFromKeys( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set geometry from keys", "", "", "Set geometry from keys." );

    CAF_PDM_InitScriptableFieldNoDefault( &m_verticesKey, "VerticesKey", "", "", "", "Key for x, y and depth of each vertex" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_trianglesKey, "TrianglesKey", "", "", "", "Key for three vertex indices per triangle" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcTriangulatedSurface_setGeometryFromKeys::execute()
{
    RimTriangulatedSurface* surface = self<RimTriangulatedSurface>();
    if ( !surface ) return std::unexpected( "No surface found" );

    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    std::vector<float> vertexValues   = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_verticesKey().toStdString() ) );
    std::vector<float> triangleValues = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_trianglesKey().toStdString() ) );

    keyValueStore->remove( m_verticesKey().toStdString() );
    keyValueStore->remove( m_trianglesKey().toStdString() );

    if ( vertexValues.empty() || vertexValues.size() % 3 != 0 )
        return std::unexpected( "Invalid vertices. Must be a non-empty list of x, y and depth values." );
    if ( triangleValues.empty() || triangleValues.size() % 3 != 0 )
        return std::unexpected( "Invalid triangles. Must be a non-empty list of three vertex indices per triangle." );

    std::vector<cvf::Vec3d> vertices;
    for ( size_t i = 0; i < vertexValues.size(); i += 3 )
    {
        // Depth is positive downwards
        vertices.emplace_back( vertexValues[i], vertexValues[i + 1], -vertexValues[i + 2] );
    }

    // The key-value store holds floats, so the indices arrive as floats
    std::vector<unsigned> triangleIndices;
    for ( float value : triangleValues )
    {
        if ( value < 0.0f || value >= static_cast<float>( vertices.size() ) || std::floor( value ) != value )
            return std::unexpected( QString( "Invalid triangle index %1. Valid range: [0-%2]" )
                                        .arg( static_cast<double>( value ) )
                                        .arg( static_cast<int>( vertices.size() ) - 1 ) );

        triangleIndices.push_back( static_cast<unsigned>( value ) );
    }

    surface->setGeometry( vertices, triangleIndices );
    surface->onLoadData();

    if ( auto coll = surface->firstAncestorOrThisOfType<RimSurfaceCollection>() )
    {
        coll->updateUiIconFromToggleField();
        coll->updateViews();
    }

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimTriangulatedSurface, RimcTriangulatedSurface_setPropertyFromKey, "SetPropertyFromKey" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcTriangulatedSurface_setPropertyFromKey::RimcTriangulatedSurface_setPropertyFromKey( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Set property from key", "", "", "Set property from key." );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "", "", "", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_valueKey, "ValueKey", "", "", "", "Key Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcTriangulatedSurface_setPropertyFromKey::execute()
{
    RimTriangulatedSurface* surface = self<RimTriangulatedSurface>();
    if ( !surface ) return std::unexpected( "No surface found" );

    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    std::vector<float> values = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_valueKey().toStdString() ) );
    keyValueStore->remove( m_valueKey().toStdString() );

    if ( values.empty() ) return std::unexpected( "Found unexpected empty property." );

    if ( values.size() != surface->vertexCount() )
        return std::unexpected( "Failed to set property: incorrect dimensions. Must be one value per vertex." );

    surface->setProperty( m_name, values );

    surface->onLoadData();

    if ( auto coll = surface->firstAncestorOrThisOfType<RimSurfaceCollection>() )
    {
        coll->updateUiIconFromToggleField();
        coll->updateViews();
    }

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimTriangulatedSurface, RimcTriangulatedSurface_getPropertyToKey, "GetPropertyToKey" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcTriangulatedSurface_getPropertyToKey::RimcTriangulatedSurface_getPropertyToKey( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Get property to key", "", "", "Get property to key." );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "", "", "", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_valueKey, "ValueKey", "", "", "", "Key Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcTriangulatedSurface_getPropertyToKey::execute()
{
    RimTriangulatedSurface* surface = self<RimTriangulatedSurface>();
    if ( !surface ) return std::unexpected( "No surface found" );

    std::vector<float> values = surface->getProperty( m_name );
    if ( values.empty() ) return std::unexpected( QString( "Property '%1' not found." ).arg( m_name() ) );

    auto keyValueStore = RiaApplication::instance()->keyValueStore();
    keyValueStore->set( m_valueKey().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( values ) );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimTriangulatedSurface, RimcTriangulatedSurface_propertyNames, "PropertyNames" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcTriangulatedSurface_propertyNames::RimcTriangulatedSurface_propertyNames( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Property Names", "", "", "Property Names." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcTriangulatedSurface_propertyNames::execute()
{
    RimTriangulatedSurface* surface = self<RimTriangulatedSurface>();
    if ( !surface ) return std::unexpected( "No surface found" );

    auto dataObject            = new RimcDataContainerString();
    dataObject->m_stringValues = surface->propertyNames();

    return dataObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcTriangulatedSurface_propertyNames::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}
