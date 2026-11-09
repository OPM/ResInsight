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

#include "RimCellFilter.h"
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
                        "Create a well target mapping and compute it. Parameters left at their default value use the same defaults "
                        "as the user interface." );

    CAF_PDM_InitScriptableField( &m_name, "Name", QString(), "", "", "", "Name of the well target mapping. Automatically generated if empty" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", -1, "", "", "", "Time step index. Use -1 for the last time step" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_volumeType, "VolumeType", "" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_volumeResultType, "VolumeResultType", "" );
    CAF_PDM_InitScriptableField( &m_volumesType,
                                 "VolumesType",
                                 QString(),
                                 "",
                                 "",
                                 "",
                                 "One of RESERVOIR_VOLUMES, SURFACE_VOLUMES_SFIP, SURFACE_VOLUMES_FIP, RESERVOIR_VOLUMES_COMPUTED. "
                                 "Picked from the available results if empty" );

    CAF_PDM_InitScriptableField( &m_oilFloodingType,
                                 "OilFloodingType",
                                 caf::AppEnum<RigFloodingSettings::FloodingType>( RigFloodingSettings::FloodingType::WATER_FLOODING ),
                                 "" );
    CAF_PDM_InitScriptableField( &m_userDefinedFloodingOil, "UserDefinedFloodingOil", 0.0, "" );
    CAF_PDM_InitScriptableField( &m_gasFloodingType,
                                 "GasFloodingType",
                                 caf::AppEnum<RigFloodingSettings::FloodingType>( RigFloodingSettings::FloodingType::GAS_FLOODING ),
                                 "",
                                 "",
                                 "",
                                 "GAS_FLOODING or USER_DEFINED" );
    CAF_PDM_InitScriptableField( &m_userDefinedFloodingGas, "UserDefinedFloodingGas", 0.0, "" );

    const QString minimumComment = "Minimum cell value. Use a negative value for the default computed from the case";
    CAF_PDM_InitScriptableField( &m_minimumSaturationOil, "MinimumSaturationOil", -1.0, "", "", "", minimumComment );
    CAF_PDM_InitScriptableField( &m_minimumSaturationGas, "MinimumSaturationGas", -1.0, "", "", "", minimumComment );
    CAF_PDM_InitScriptableField( &m_minimumPressure, "MinimumPressure", -1.0, "", "", "", minimumComment );
    CAF_PDM_InitScriptableField( &m_minimumPermeability, "MinimumPermeability", -1.0, "", "", "", minimumComment );
    CAF_PDM_InitScriptableField( &m_minimumTransmissibility, "MinimumTransmissibility", -1.0, "", "", "", minimumComment );

    CAF_PDM_InitScriptableField( &m_maxNumTargets, "MaxNumTargets", 5, "", "", "", "Maximum number of well targets (clusters)" );
    CAF_PDM_InitScriptableField( &m_maxIterations, "MaxIterations", 100000, "", "", "", "Maximum number of iterations per cluster" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_startDataFilter,
                                          "StartDataFilter",
                                          "",
                                          "",
                                          "",
                                          "Data filter limiting where clusters can start. Uses the grow data filter if empty" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_growDataFilter, "GrowDataFilter", "", "", "", "Data filter limiting where clusters can grow" );
    CAF_PDM_InitScriptableField( &m_avoidFaults, "AvoidFaults", false, "", "", "", "Do not grow clusters across faults" );

    const QString ensembleComment = "Only used for ensembles";
    CAF_PDM_InitScriptableField( &m_cellCountI, "CellCountI", 100, "", "", "", ensembleComment );
    CAF_PDM_InitScriptableField( &m_cellCountJ, "CellCountJ", 100, "", "", "", ensembleComment );
    CAF_PDM_InitScriptableField( &m_cellCountK, "CellCountK", 10, "", "", "", ensembleComment );
    CAF_PDM_InitScriptableField( &m_expandBoundingBoxXYPercent, "ExpandBoundingBoxXyPercent", 5.0, "", "", "", ensembleComment );
    CAF_PDM_InitScriptableField( &m_expandBoundingBoxZPercent, "ExpandBoundingBoxZPercent", 10.0, "", "", "", ensembleComment );
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

    auto deleteAndFail = [wellTargetMapping]( const QString& message ) -> std::unexpected<QString>
    {
        if ( auto parentField = wellTargetMapping->parentField() ) parentField->removeChild( wellTargetMapping );
        delete wellTargetMapping;
        return std::unexpected( message );
    };

    if ( auto result = applyParameters( wellTargetMapping ); !result ) return deleteAndFail( result.error() );
    if ( auto result = wellTargetMapping->compute(); !result ) return deleteAndFail( result.error() );

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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<void, QString> RimcWellTargetMapping_addBase::applyParameters( RimWellTargetMapping* wellTargetMapping ) const
{
    RimWellTargetMapping::Settings settings;

    if ( m_timeStep() >= 0 ) settings.timeStep = m_timeStep();

    settings.volumeType       = m_volumeType();
    settings.volumeResultType = m_volumeResultType();

    if ( !m_volumesType().isEmpty() )
    {
        using VolumesTypeEnum = caf::AppEnum<RigWellTargetMapping::VolumesType>;
        if ( !VolumesTypeEnum::isValid( m_volumesType() ) )
        {
            QStringList validTexts;
            for ( size_t i = 0; i < VolumesTypeEnum::size(); i++ )
                validTexts.push_back( VolumesTypeEnum::textFromIndex( i ) );

            return std::unexpected(
                QString( "Invalid volumes type '%1'. Valid values: %2" ).arg( m_volumesType() ).arg( validTexts.join( ", " ) ) );
        }
        settings.volumesType = VolumesTypeEnum::fromText( m_volumesType() );
    }

    settings.oilFloodingType        = m_oilFloodingType();
    settings.userDefinedFloodingOil = m_userDefinedFloodingOil();
    settings.gasFloodingType        = m_gasFloodingType();
    settings.userDefinedFloodingGas = m_userDefinedFloodingGas();

    auto valueIfSet                  = []( double value ) { return value >= 0.0 ? std::optional<double>( value ) : std::nullopt; };
    settings.minimumSaturationOil    = valueIfSet( m_minimumSaturationOil() );
    settings.minimumSaturationGas    = valueIfSet( m_minimumSaturationGas() );
    settings.minimumPressure         = valueIfSet( m_minimumPressure() );
    settings.minimumPermeability     = valueIfSet( m_minimumPermeability() );
    settings.minimumTransmissibility = valueIfSet( m_minimumTransmissibility() );

    settings.maxNumTargets   = m_maxNumTargets();
    settings.maxIterations   = m_maxIterations();
    settings.startDataFilter = m_startDataFilter();
    settings.growDataFilter  = m_growDataFilter();
    settings.avoidFaults     = m_avoidFaults();

    settings.cellCountI                 = m_cellCountI();
    settings.cellCountJ                 = m_cellCountJ();
    settings.cellCountK                 = m_cellCountK();
    settings.expandBoundingBoxXYPercent = m_expandBoundingBoxXYPercent();
    settings.expandBoundingBoxZPercent  = m_expandBoundingBoxZPercent();

    wellTargetMapping->applySettings( settings );
    if ( !m_name().isEmpty() ) wellTargetMapping->setName( m_name() );

    return {};
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
