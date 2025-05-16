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
#include "RimcRegularSurface.h"

#include "RiaApplication.h"

#include "RimRegularSurface.h"

#include "RigSurface.h"

#include "RifSurfaceExporter.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimRegularSurface, RimcRegularSurface_setPropertyFromKey, "SetPropertyFromKey" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcRegularSurface_setPropertyFromKey::RimcRegularSurface_setPropertyFromKey( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Set property from key", "", "", "Set property from key." );
    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "", "", "", "Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_valueKey, "ValueKey", "", "", "", "Key Value" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcRegularSurface_setPropertyFromKey::execute()
{
    RimRegularSurface* surface = self<RimRegularSurface>();
    if ( !surface ) return std::unexpected( "No surface found" );

    auto keyValueStore = RiaApplication::instance()->keyValueStore();

    // TODO: duplicated with corner point case
    auto convertToFloatVector = []( const std::optional<std::vector<char>>& input ) -> std::vector<float>
    {
        if ( input && !input->empty() )
        {
            // Ensure the byte vector size is a multiple of sizeof(float)
            if ( input->size() % sizeof( float ) != 0 )
            {
                return {};
            }

            // Calculate how many floats we'll have
            size_t float_count = input->size() / sizeof( float );

            // Create a vector of floats with the appropriate size
            std::vector<float> float_vec( float_count );

            // Copy the binary data from the byte vector to the float vector
            std::memcpy( float_vec.data(), input->data(), input->size() );

            return float_vec;
        }

        return {};
    };

    std::vector<float> values = convertToFloatVector( keyValueStore->get( m_valueKey().toStdString() ) );
    if ( values.empty() ) return std::unexpected( "Found unexcepted empty property." );

    RigSurface* surfaceData = surface->surfaceData();
    surfaceData->addVertexResult( m_name(), values );
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcRegularSurface_setPropertyFromKey::resultIsPersistent() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcRegularSurface_setPropertyFromKey::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcRegularSurface_setPropertyFromKey::isNullptrValidResult() const
{
    return true;
}
