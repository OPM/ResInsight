/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-  Equinor ASA
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

#include "RimcWellTargetMapping.h"

#include "RimEclipseCase.h"
#include "RimEclipseCaseEnsemble.h"
#include "RimReservoirGridEnsemble.h"
#include "RimWellTargetMapping.h"

#include "cafPdmFieldScriptingCapability.h"

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimWellTargetMapping, RimcWellTargetMapping_compute, "Compute" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellTargetMapping_compute::RimcWellTargetMapping_compute( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Compute", "", "", "Compute the well target mapping using the current settings" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellTargetMapping_compute::execute()
{
    auto* wellTargetMapping = self<RimWellTargetMapping>();
    if ( !wellTargetMapping ) return std::unexpected( "No well target mapping" );

    if ( auto result = wellTargetMapping->compute(); !result ) return std::unexpected( result.error() );

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcWellTargetMapping_addBase::RimcWellTargetMapping_addBase( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Add Well Target Mapping",
                        "",
                        "",
                        "Create a well target mapping with the same defaults as the user interface. Set fields on the returned "
                        "object, call update() and then compute()." );

    CAF_PDM_InitScriptableField( &m_name, "Name", QString(), "", "", "", "Name of the well target mapping. Automatically generated if empty" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimcWellTargetMapping_addBase::execute()
{
    if ( auto valid = validateParent(); !valid ) return std::unexpected( valid.error() );

    auto* wellTargetMapping = new RimWellTargetMapping();
    addToParent( wellTargetMapping );
    wellTargetMapping->setDefaults();
    if ( !m_name().isEmpty() ) wellTargetMapping->setName( m_name() );

    if ( auto parent = wellTargetMapping->parentField() ? wellTargetMapping->parentField()->ownerObject() : nullptr )
    {
        parent->uiCapability()->updateConnectedEditors();
    }

    return wellTargetMapping;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimcWellTargetMapping_addBase::classKeywordReturnedType() const
{
    return RimWellTargetMapping::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimEclipseCase, RimcEclipseCase_addWellTargetMapping, "AddWellTargetMapping" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcEclipseCase_addWellTargetMapping::RimcEclipseCase_addWellTargetMapping( caf::PdmObjectHandle* self )
    : RimcWellTargetMapping_addBase( self )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimcEclipseCase_addWellTargetMapping::validateParent() const
{
    // Same restriction as the GUI: mappings for ensemble cases belong to the ensemble
    auto eclipseCase = self<RimEclipseCase>();
    if ( eclipseCase->ensemble() || eclipseCase->firstAncestorOrThisOfType<RimEclipseCaseEnsemble>() )
    {
        return std::unexpected( "Well target mapping is not supported for cases in an ensemble. Use "
                                "ReservoirGridEnsemble.add_well_target_mapping for reservoir grid ensembles." );
    }
    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimcEclipseCase_addWellTargetMapping::addToParent( RimWellTargetMapping* wellTargetMapping )
{
    self<RimEclipseCase>()->addWellTargetMapping( wellTargetMapping );
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimReservoirGridEnsemble, RimcReservoirGridEnsemble_addWellTargetMapping, "AddWellTargetMapping" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimcReservoirGridEnsemble_addWellTargetMapping::RimcReservoirGridEnsemble_addWellTargetMapping( caf::PdmObjectHandle* self )
    : RimcWellTargetMapping_addBase( self )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimcReservoirGridEnsemble_addWellTargetMapping::addToParent( RimWellTargetMapping* wellTargetMapping )
{
    self<RimReservoirGridEnsemble>()->addWellTargetMapping( wellTargetMapping );
}
