/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026- Equinor ASA
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

#include "RimcGeoMechCase.h"

#include "WellLogCommands/RicNewWellBoreStabilityPlotFeature.h"

#include "KeyValueStore/RiaKeyValueStoreUtil.h"
#include "RiaApplication.h"

#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimWbsParameters.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellPath.h"
#include "RimcDataContainerString.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmScriptEnumNameRegistry.h"

namespace
{
struct RegisterScriptEnumNames
{
    RegisterScriptEnumNames() { caf::PdmScriptEnumNameRegistry::registerName<RigFemResultPosEnum>( "GeoMechResultPosition" ); }
};
const RegisterScriptEnumNames s_registerScriptEnumNames;
} // namespace

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimGeoMechCase, RimGeoMechCase_createWellBoreStabilityPlot, "createWellBoreStabilityPlot" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase_createWellBoreStabilityPlot::RimGeoMechCase_createWellBoreStabilityPlot( caf::PdmObjectHandle* self )
    : caf::PdmObjectCreationMethod( self )
{
    CAF_PDM_InitObject( "Create Well Bore Stability Plot", "", "", "Create a Well Bore Stability plot for a well path in this case" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellPath, "WellPath", "Well Path", "", "", "Well path to create the plot for" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "Zero-based time step index" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase_createWellBoreStabilityPlot::setWellPath( RimWellPath* wellPath )
{
    m_wellPath = wellPath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase_createWellBoreStabilityPlot::setTimeStep( int timeStep )
{
    m_timeStep = timeStep;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimGeoMechCase_createWellBoreStabilityPlot::setParameters( const RimWbsParameters* parameters )
{
    m_parameters = parameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimGeoMechCase_createWellBoreStabilityPlot::execute()
{
    auto* geoMechCase = self<RimGeoMechCase>();
    if ( !geoMechCase ) return std::unexpected( "No GeoMech case is available." );

    RimWellPath* wellPath = m_wellPath();
    if ( !wellPath ) return std::unexpected( "No well path specified." );

    if ( !wellPath->wellPathGeometry() )
    {
        return std::unexpected(
            QString( "The well path %1 has no geometry. Cannot create a Well Bore Stability Plot" ).arg( wellPath->name() ) );
    }

    if ( m_timeStep() < 0 ) return std::unexpected( "Time step must be non-negative." );

    RimWellBoreStabilityPlot* plot = RicNewWellBoreStabilityPlotFeature::createPlot( geoMechCase, wellPath, m_timeStep(), m_parameters );
    if ( !plot ) return std::unexpected( "Could not create Well Bore Stability plot." );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase_createWellBoreStabilityPlot::classKeywordReturnedType() const
{
    return RimWellBoreStabilityPlot::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimGeoMechCase, RimGeoMechCase_timeStepStrings, "TimeStepStringsInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase_timeStepStrings::RimGeoMechCase_timeStepStrings( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Time Step Strings", "", "", "Time step strings for this case." );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimGeoMechCase_timeStepStrings::execute()
{
    auto* geoMechCase = self<RimGeoMechCase>();
    if ( !geoMechCase ) return std::unexpected( "No GeoMech case is available." );

    auto* container = new RimcDataContainerString();

    const QStringList names   = geoMechCase->timeStepStrings();
    container->m_stringValues = std::vector<QString>( names.begin(), names.end() );

    return container;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase_timeStepStrings::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimGeoMechCase, RimGeoMechCase_resultNames, "ResultNamesInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase_resultNames::RimGeoMechCase_resultNames( caf::PdmObjectHandle* self )
    : PdmObjectMethod( self, PdmObjectMethod::NullPointerType::NULL_IS_INVALID, PdmObjectMethod::ResultType::PERSISTENT_FALSE )
{
    CAF_PDM_InitObject( "Result Names", "", "", "Result names for this case." );

    CAF_PDM_InitScriptableField( &m_resultPosition,
                                 "ResultPosition",
                                 caf::AppEnum<RigFemResultPosEnum>( RIG_ELEMENT_NODAL ),
                                 "Result Position",
                                 "",
                                 "",
                                 "Result position, for example NODAL or ELEMENT_NODAL" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimGeoMechCase_resultNames::execute()
{
    auto* geoMechCase = self<RimGeoMechCase>();
    if ( !geoMechCase ) return std::unexpected( "No GeoMech case is available." );

    if ( !geoMechCase->geoMechData() || !geoMechCase->geoMechData()->femPartResults() )
    {
        return std::unexpected( "The GeoMech case has no result data. Open the case first." );
    }

    const auto fieldsWithComponents = geoMechCase->geoMechData()->femPartResults()->scalarFieldAndComponentNames( m_resultPosition() );

    std::vector<QString> names;
    for ( const auto& [field, components] : fieldsWithComponents )
    {
        const QString fieldName = QString::fromStdString( field );
        if ( components.empty() )
        {
            names.push_back( fieldName );
            continue;
        }

        for ( const auto& component : components )
        {
            names.push_back( fieldName + ":" + QString::fromStdString( component ) );
        }
    }

    auto* container           = new RimcDataContainerString();
    container->m_stringValues = std::move( names );

    return container;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimGeoMechCase_resultNames::classKeywordReturnedType() const
{
    return RimcDataContainerString::classKeywordStatic();
}

CAF_PDM_OBJECT_METHOD_SOURCE_INIT( RimGeoMechCase, RimGeoMechCase_readResultValuesInternal, "ReadResultValuesInternal" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGeoMechCase_readResultValuesInternal::RimGeoMechCase_readResultValuesInternal( caf::PdmObjectHandle* self )
    : caf::PdmVoidObjectMethod( self )
{
    CAF_PDM_InitObject( "Read Result Values", "", "", "Read GeoMech result values" );

    CAF_PDM_InitScriptableField( &m_resultPosition,
                                 "ResultPosition",
                                 caf::AppEnum<RigFemResultPosEnum>( RIG_ELEMENT_NODAL ),
                                 "Result Position",
                                 "",
                                 "",
                                 "Result position, for example NODAL or ELEMENT_NODAL" );
    CAF_PDM_InitScriptableField( &m_fieldName, "FieldName", QString(), "Field Name" );
    CAF_PDM_InitScriptableField( &m_componentName, "ComponentName", QString(), "Component Name" );
    CAF_PDM_InitScriptableField( &m_timeStep, "TimeStep", 0, "Time Step", "", "", "Zero-based time step index" );
    CAF_PDM_InitScriptableField( &m_frameIndex, "FrameIndex", -1, "Frame Index", "", "", "Zero-based frame index. Negative selects the last frame" );
    CAF_PDM_InitScriptableField( &m_key, "Key", QString(), "Key" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::expected<caf::PdmObjectHandle*, QString> RimGeoMechCase_readResultValuesInternal::execute()
{
    auto* geoMechCase = self<RimGeoMechCase>();
    if ( !geoMechCase ) return std::unexpected( "No GeoMech case is available." );

    if ( m_key().isEmpty() ) return std::unexpected( "No key specified." );
    if ( m_fieldName().isEmpty() ) return std::unexpected( "No result field name specified." );

    if ( !geoMechCase->geoMechData() || !geoMechCase->geoMechData()->femPartResults() )
    {
        return std::unexpected( "The GeoMech case has no result data. Open the case first." );
    }

    auto* results = geoMechCase->geoMechData()->femPartResults();

    if ( m_timeStep() < 0 || m_timeStep() >= results->timeStepCount() )
    {
        return std::unexpected( QString( "Time step %1 is out of range [0, %2)." ).arg( m_timeStep() ).arg( results->timeStepCount() ) );
    }

    const int frameCount = results->frameCount( m_timeStep() );
    const int frameIndex = m_frameIndex() < 0 ? frameCount - 1 : m_frameIndex();
    if ( frameIndex < 0 || frameIndex >= frameCount )
    {
        return std::unexpected( QString( "Frame %1 is out of range [0, %2)." ).arg( m_frameIndex() ).arg( frameCount ) );
    }

    const RigFemResultAddress address( m_resultPosition(), m_fieldName().toStdString(), m_componentName().toStdString() );
    if ( !address.isValid() ) return std::unexpected( QString( "Invalid result %1 %2." ).arg( m_fieldName(), m_componentName() ) );

    std::vector<float> values;
    results->globalResultValues( address, m_timeStep(), frameIndex, values );

    RiaApplication::instance()->keyValueStore()->set( m_key().toStdString(), RiaKeyValueStoreUtil::convertToByteVector( values ) );

    return nullptr;
}
