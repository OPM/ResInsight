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

#include "RimcGeoGuidedWellPlacement.h"

#include "RimGeoGuidedWellPlacement.h"
#include "RimReservoirGridEnsemble.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimGeoGuidedWellPlacement, RimcGeoGuidedWellPlacement_compute, "Compute" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcGeoGuidedWellPlacement_compute::RimcGeoGuidedWellPlacement_compute( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Compute",
                        "",
                        "",
                        "Create one well path per realization using the current settings. Well paths created by a previous "
                        "compute are deleted." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcGeoGuidedWellPlacement_compute::execute()
{
    auto* placement = self<RimGeoGuidedWellPlacement>();
    if ( !placement ) return std::unexpected( "No geo-guided well placement" );

    if ( auto result = placement->compute(); !result ) return std::unexpected( result.error() );

    return nullptr;
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimReservoirGridEnsemble, RimcReservoirGridEnsemble_addGeoGuidedWellPlacement, "AddGeoGuidedWellPlacement" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcReservoirGridEnsemble_addGeoGuidedWellPlacement::RimcReservoirGridEnsemble_addGeoGuidedWellPlacement( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Geo-Guided Well Placement",
                        "",
                        "",
                        "Create a geo-guided well placement with the same defaults as the user interface. Set fields on the "
                        "returned object, call update() and then compute()." );

    CAF_PDM_InitScriptableField( &m_name, "Name", QString(), "", "", "", "Name of the well placement. Automatically generated if empty" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcReservoirGridEnsemble_addGeoGuidedWellPlacement::execute()
{
    auto ensemble = self<RimReservoirGridEnsemble>();

    auto* placement = new RimGeoGuidedWellPlacement();
    ensemble->addGeoGuidedWellPlacement( placement );
    placement->setDefaults();
    if ( !m_name().isEmpty() ) placement->setName( m_name() );

    ensemble->updateConnectedEditors();

    return placement;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcReservoirGridEnsemble_addGeoGuidedWellPlacement::classKeywordReturnedType() const
{
    return RimGeoGuidedWellPlacement::classKeywordStatic();
}
