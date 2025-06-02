/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025 Equinor ASA
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

#include "RimcFishbonesCollection.h"

#include "RiaLogging.h"

#include "RimFishbones.h"
#include "RimFishbonesCollection.h"
#include "RimWellPath.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFishbonesCollection, RimcFishbonesCollection_appendFishbones, "AppendFishbones" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFishbonesCollection_appendFishbones::RimcFishbonesCollection_appendFishbones( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Append Fishbones", "", "", "Append Fishbones" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_subLocations, "SubLocations", "SubLocations" );
    CAF_PDM_InitScriptableField( &m_drillingType, "DrillingType", RimFishbonesDefines::DrillingType::STANDARD, "DrillingType" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFishbonesCollection_appendFishbones::execute()
{
    auto fishbonesCollection = self<RimFishbonesCollection>();
    if ( !fishbonesCollection ) return std::unexpected( "No fishbones collection found" );

    if ( m_subLocations().empty() )
    {
        return std::unexpected(
            "Sub locations are empty, expected list of float values defining measured depths. Cannot create fishbones object." );
    }

    auto* fishbonesObject = fishbonesCollection->appendFishbonesSubsAtLocations( m_subLocations(), m_drillingType() );

    if ( auto wellPath = fishbonesCollection->firstAncestorOfType<RimWellPath>() )
    {
        wellPath->updateAllRequiredEditors();
    }

    return fishbonesObject;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFishbonesCollection_appendFishbones::resultIsPersistent_obsolete() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcFishbonesCollection_appendFishbones::defaultResult() const
{
    return std::unique_ptr<caf::PdmObjectHandle>( new RimFishbones );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFishbonesCollection, RimcFishbonesCollection_setFixedStartLocation, "SetFixedStartLocation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFishbonesCollection_setFixedStartLocation::RimcFishbonesCollection_setFixedStartLocation( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Fixed Start Location", "", "", "" );

    CAF_PDM_InitScriptableField( &m_location, "Location", 0.0, "Location" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFishbonesCollection_setFixedStartLocation::execute()
{
    auto fishbonesCollection = self<RimFishbonesCollection>();
    if ( fishbonesCollection )
    {
        fishbonesCollection->setFixedStartMD( m_location() );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFishbonesCollection_setFixedStartLocation::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcFishbonesCollection_setFixedStartLocation::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFishbonesCollection_setFixedStartLocation::isNullptrValidResult_obsolete() const
{
    return true;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimFishbonesCollection, RimcFishbonesCollection_setFixedEndLocation, "SetFixedEndLocation" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcFishbonesCollection_setFixedEndLocation::RimcFishbonesCollection_setFixedEndLocation( caf::PdmObjectHandle* self )
    : caf::PdmObjectMethod( self )
{
    CAF_PDM_InitObject( "Set Fixed End Location", "", "", "" );

    CAF_PDM_InitScriptableField( &m_location, "Location", 0.0, "Location" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcFishbonesCollection_setFixedEndLocation::execute()
{
    auto fishbonesCollection = self<RimFishbonesCollection>();
    if ( fishbonesCollection )
    {
        fishbonesCollection->setFixedEndMD( m_location() );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFishbonesCollection_setFixedEndLocation::resultIsPersistent_obsolete() const
{
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::unique_ptr<caf::PdmObjectHandle> RimcFishbonesCollection_setFixedEndLocation::defaultResult() const
{
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimcFishbonesCollection_setFixedEndLocation::isNullptrValidResult_obsolete() const
{
    return true;
}
