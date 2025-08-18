/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RimValveTemplate.h"

#include "RimWellPathAicdParameters.h"
#include "RimWellPathValve.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiTreeOrdering.h"

#include "opm/input/eclipse/Parser/ParserKeywords/W.hpp"

CAF_PDM_SOURCE_INIT( RimValveTemplate, "ValveTemplate" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplate::RimValveTemplate()
{
    CAF_PDM_InitScriptableObject( "Valve Template", ":/ICDValve16x16.png" );

    CAF_PDM_InitField( &m_valveTemplateUnit,
                       "UnitSystem",
                       caf::AppEnum<RiaDefines::EclipseUnitSystem>( RiaDefines::EclipseUnitSystem::UNITS_UNKNOWN ),
                       "Units System" );
    m_valveTemplateUnit.uiCapability()->setUiReadOnly( true );

    CAF_PDM_InitFieldNoDefault( &m_type, "CompletionType", "Type" );
    m_type = RiaDefines::WellPathComponentType::ICD;
    CAF_PDM_InitField( &m_userLabel, "UserLabel", QString( "Template" ), "Name" );

    setName( fullLabel() );

    CAF_PDM_InitScriptableField( &m_orificeDiameter, "OrificeDiameter", 8.0, "Orifice Diameter [mm]" );
    CAF_PDM_InitScriptableField( &m_flowCoefficient, "FlowCoefficient", 0.7, "Flow Coefficient" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_aicdParameters, "AICDParameters", "AicdParameters", "AICD Parameters" );
    m_aicdParameters = new RimWellPathAicdParameters;
    m_aicdParameters.uiCapability()->setUiTreeChildrenHidden( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplate::~RimValveTemplate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::loadDataAndUpdate()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::setUnitSystem( caf::AppEnum<RiaDefines::EclipseUnitSystem> unitSystem )
{
    m_valveTemplateUnit = unitSystem;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::setDefaultValuesFromUnits()
{
    if ( m_valveTemplateUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
    {
        m_orificeDiameter = 8;
    }
    else if ( m_valveTemplateUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
    {
        m_orificeDiameter = 0.315;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::WellPathComponentType RimValveTemplate::type() const
{
    return m_type();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::setType( RiaDefines::WellPathComponentType type )
{
    CAF_ASSERT( type == RiaDefines::WellPathComponentType::ICD || type == RiaDefines::WellPathComponentType::AICD ||
                type == RiaDefines::WellPathComponentType::ICV );

    m_type = type;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::AppEnum<RiaDefines::EclipseUnitSystem> RimValveTemplate::templateUnits() const
{
    return m_valveTemplateUnit;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimValveTemplate::orificeDiameter() const
{
    return m_orificeDiameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimValveTemplate::flowCoefficient() const
{
    return m_flowCoefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const RimWellPathAicdParameters* RimValveTemplate::aicdParameters() const
{
    return m_aicdParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimValveTemplate::typeLabel() const
{
    return m_type().uiText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimValveTemplate::fullLabel() const
{
    QString label = QString( "%1: %2" ).arg( typeLabel() ).arg( m_userLabel() );
    return label;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::setUserLabel( const QString& userLabel )
{
    m_userLabel = userLabel;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::setOrificeDiameter( double diameter )
{
    m_orificeDiameter = diameter;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::setFlowCoefficient( double coefficient )
{
    m_flowCoefficient = coefficient;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::setAicdParameter( AICDParameters parameter, double value )
{
    if ( m_aicdParameters() )
    {
        m_aicdParameters()->setValue( parameter, value );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimValveTemplate* RimValveTemplate::createAicdTemplate( const RiaOpmParserTools::AicdTemplateValues& aicdParameters, int templateNumber )
{
    RimValveTemplate* aicdTemplate = new RimValveTemplate;
    aicdTemplate->setType( RiaDefines::WellPathComponentType::AICD );

    QString name;
    if ( aicdParameters.contains( RiaOpmParserTools::aicdTemplateId() ) )
    {
        auto id = aicdParameters.at( RiaOpmParserTools::aicdTemplateId() );
        name    = QString::number( id );
    }
    else
    {
        name = QString::number( templateNumber );
    }

    aicdTemplate->setUserLabel( name );

    using namespace Opm::ParserKeywords;
    std::map<std::string, AICDParameters> parameterMap = { { WSEGAICD::STRENGTH::itemName, AICD_STRENGTH },
                                                           { WSEGAICD::DENSITY_CALI::itemName, AICD_DENSITY_CALIB_FLUID },
                                                           { WSEGAICD::VISCOSITY_CALI::itemName, AICD_VISCOSITY_CALIB_FLUID },
                                                           { WSEGAICD::FLOW_RATE_EXPONENT::itemName, AICD_VOL_FLOW_EXP },
                                                           { WSEGAICD::VISC_EXPONENT::itemName, AICD_VISOSITY_FUNC_EXP },
                                                           { WSEGAICD::CRITICAL_VALUE::itemName, AICD_CRITICAL_WATER_IN_LIQUID_FRAC },
                                                           { WSEGAICD::MAX_ABS_RATE::itemName, AICD_MAX_FLOW_RATE },
                                                           { WSEGAICD::OIL_FLOW_FRACTION::itemName, AICD_EXP_OIL_FRAC_DENSITY },
                                                           { WSEGAICD::WATER_FLOW_FRACTION::itemName, AICD_EXP_WATER_FRAC_DENSITY },
                                                           { WSEGAICD::GAS_FLOW_FRACTION::itemName, AICD_EXP_GAS_FRAC_DENSITY },
                                                           { WSEGAICD::OIL_VISC_FRACTION::itemName, AICD_EXP_OIL_FRAC_VISCOSITY },
                                                           { WSEGAICD::WATER_VISC_FRACTION::itemName, AICD_EXP_WATER_FRAC_VISCOSITY },
                                                           { WSEGAICD::GAS_VISC_FRACTION::itemName, AICD_EXP_GAS_FRAC_VISCOSITY } };

    for ( const auto& parameter : parameterMap )
    {
        if ( aicdParameters.contains( parameter.first ) )
        {
            auto incomingValue = aicdParameters.at( parameter.first );
            aicdTemplate->setAicdParameter( parameter.second, incomingValue );
        }
    }

    return aicdTemplate;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimValveTemplate::defaultOrificeDiameter()
{
    return 8.0;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
double RimValveTemplate::defaultFlowCoefficient()
{
    return 0.7;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimValveTemplate::calculateValueOptions( const caf::PdmFieldHandle* fieldNeedingOptions )
{
    QList<caf::PdmOptionItemInfo> options;

    if ( fieldNeedingOptions == &m_type )
    {
        std::set<RiaDefines::WellPathComponentType> supportedTypes = { RiaDefines::WellPathComponentType::ICD,
                                                                       RiaDefines::WellPathComponentType::AICD,
                                                                       RiaDefines::WellPathComponentType::ICV };
        for ( RiaDefines::WellPathComponentType type : supportedTypes )
        {
            options.push_back( caf::PdmOptionItemInfo( CompletionTypeEnum::uiText( type ), type ) );
        }
    }
    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    uiOrdering.add( &m_type );
    uiOrdering.add( &m_userLabel );
    uiOrdering.add( &m_valveTemplateUnit );
    if ( m_type() == RiaDefines::WellPathComponentType::ICV || m_type() == RiaDefines::WellPathComponentType::ICD )
    {
        if ( m_valveTemplateUnit == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            m_orificeDiameter.uiCapability()->setUiName( "Orifice Diameter [mm]" );
        }
        else if ( m_valveTemplateUnit == RiaDefines::EclipseUnitSystem::UNITS_FIELD )
        {
            m_orificeDiameter.uiCapability()->setUiName( "Orifice Diameter [in]" );
        }
    }

    if ( m_type() == RiaDefines::WellPathComponentType::ICV || m_type() == RiaDefines::WellPathComponentType::ICD )
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "MSW Valve Parameters" );
        group->add( &m_orificeDiameter );
        group->add( &m_flowCoefficient );
    }
    else
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup( "MSW AICD Parameters" );
        m_aicdParameters->uiOrdering( uiConfigName, *group );
    }

    bool readOnly = uiConfigName == QString( "InsideValve" );
    m_type.uiCapability()->setUiReadOnly( readOnly );
    m_userLabel.uiCapability()->setUiReadOnly( readOnly );
    m_orificeDiameter.uiCapability()->setUiReadOnly( readOnly );
    m_flowCoefficient.uiCapability()->setUiReadOnly( readOnly );

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::fieldChangedByUi( const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue )
{
    if ( changedField == &m_type || changedField == &m_userLabel )
    {
        setName( fullLabel() );
    }
    if ( changedField == &m_type )
    {
        std::vector<caf::PdmFieldHandle*> referringFields = referringPtrFields();
        for ( caf::PdmFieldHandle* field : referringFields )
        {
            RimWellPathValve* valve = dynamic_cast<RimWellPathValve*>( field->ownerObject() );
            valve->templateUpdated();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimValveTemplate::defineUiTreeOrdering( caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName /*= ""*/ )
{
    setName( fullLabel() );
    if ( m_type() == RiaDefines::WellPathComponentType::ICV )
    {
        setUiIconFromResourceString( ":/ICVValve16x16.png" );
    }
    else if ( m_type() == RiaDefines::WellPathComponentType::ICD )
    {
        setUiIconFromResourceString( ":/ICDValve16x16.png" );
    }
    else if ( m_type() == RiaDefines::WellPathComponentType::AICD )
    {
        setUiIconFromResourceString( ":/AICDValve16x16.png" );
    }
}
