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

#include "KeyValueStore/RiaKeyValueStoreUtil.h"
#include "RiaApplication.h"

#include "RimRegularSurface.h"
#include "RimSurfaceCollection.h"

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
    setNullptrValid( true );
    setResultPersistent( false );

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

    std::vector<float> values = RiaKeyValueStoreUtil::convertToFloatVector( keyValueStore->get( m_valueKey().toStdString() ) );
    if ( values.empty() ) return std::unexpected( "Found unexcepted empty property." );

    if ( values.size() != static_cast<size_t>( surface->nx() * surface->ny() ) )
        return std::unexpected( "Failed to set property: incorrect dimensions." );

    surface->setProperty( m_name, values );

    surface->onLoadData();

    if ( auto coll = surface->firstAncestorOrThisOfType<RimSurfaceCollection>() )
    {
        coll->updateUiIconFromToggleField();
        coll->updateViews();
    }

    keyValueStore->remove( m_valueKey().toStdString() );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcRegularSurface_setPropertyFromKey::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( nullptr );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimRegularSurface, RimcRegularSurface_setPropertyAsDepth, "SetPropertyAsDepth" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcRegularSurface_setPropertyAsDepth::RimcRegularSurface_setPropertyAsDepth( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Set property as depth", "", "", "Set property as depth." );
    setNullptrValid( true );
    setResultPersistent( false );

    CAF_PDM_InitScriptableFieldNoDefault( &m_name, "Name", "", "", "", "Name" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcRegularSurface_setPropertyAsDepth::execute()
{
    RimRegularSurface* surface = self<RimRegularSurface>();
    if ( !surface ) return std::unexpected( "No surface found" );

    if ( !surface->setPropertyAsDepth( m_name ) ) return std::unexpected( "Failed to set depth property." );

    surface->onLoadData();

    if ( auto coll = surface->firstAncestorOrThisOfType<RimSurfaceCollection>() )
    {
        coll->updateUiIconFromToggleField();
        coll->updateViews();
        coll->updateConnectedEditors();
    }
    surface->updateConnectedEditors();

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcRegularSurface_setPropertyAsDepth::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( nullptr );
}
