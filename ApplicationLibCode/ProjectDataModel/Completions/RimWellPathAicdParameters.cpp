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
#include "RimWellPathAicdParameters.h"

#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathValve.h"

#include "cafPdmDoubleStringValidator.h"
#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiDoubleValueEditor.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"

#include <limits>

CAF_PDM_SOURCE_INIT( RimWellPathAicdParameters, "WellPathAicdParameters" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAicdParameters::RimWellPathAicdParameters()
{
    CAF_PDM_InitScriptableObject( "RimWellPathAicdParameters" );

    CAF_PDM_InitScriptableField( &m_deviceOpen, "DeviceOpen", true, "Device Open?" );

    CAF_PDM_InitScriptableFieldWithScriptKeywordNoDefault( &m_aicdParameterFields[AICD_STRENGTH],
                                                           "StrengthAICD",
                                                           "StrengthAicd",
                                                           "Strength of AICD" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_DENSITY_CALIB_FLUID],
                                          "DensityCalibrationFluid",
                                          "Calibration Fluid Density (kg/m^3)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_VISCOSITY_CALIB_FLUID],
                                          "ViscosityCalibrationFluid",
                                          "Calibration Fluid Viscosity (cP)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_VOL_FLOW_EXP], "VolumeFlowRateExponent", "Volume Flow Rate Exponent" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_VISOSITY_FUNC_EXP],
                                          "ViscosityFunctionExponent",
                                          "Viscosity Function Exponent" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_CRITICAL_WATER_IN_LIQUID_FRAC],
                                          "CriticalWaterLiquidFractionEmul",
                                          "Critical Water in Liquid Fraction for emulsions" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_EMULSION_VISC_TRANS_REGION],
                                          "ViscosityTransitionRegionEmul",
                                          "Emulsion Viscosity Transition Region" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_MAX_RATIO_EMULSION_VISC],
                                          "MaxRatioOfEmulsionVisc",
                                          "Max Ratio of Emulsion to Continuous Viscosity" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_MAX_FLOW_RATE], "MaxFlowRate", "Max Flow Rate for AICD Device (m^3 / day)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_EXP_OIL_FRAC_DENSITY],
                                          "ExponentOilDensity",
                                          "Density Exponent of Oil Fraction" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_EXP_WATER_FRAC_DENSITY],
                                          "ExponentWaterDensity",
                                          "Density Exponent of Water Fraction" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_EXP_GAS_FRAC_DENSITY],
                                          "ExponentGasDensity",
                                          "Density Exponent of Gas Fraction" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_EXP_OIL_FRAC_VISCOSITY],
                                          "ExponentOilViscosity",
                                          "Viscosity Exponent of Oil Fraction" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_EXP_WATER_FRAC_VISCOSITY],
                                          "ExponentWaterViscosity",
                                          "Viscosity Exponent of Water Fraction" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_aicdParameterFields[AICD_EXP_GAS_FRAC_VISCOSITY],
                                          "ExponentGasViscosity",
                                          "Viscosity Exponent of Gas Fraction" );

    std::vector<caf::PdmFieldHandle*> allFields = fields();
    for ( caf::PdmFieldHandle* field : allFields )
    {
        caf::PdmField<std::optional<double>>* doubleField = dynamic_cast<caf::PdmField<std::optional<double>>*>( field );
        if ( doubleField )
        {
            doubleField->uiCapability()->setUiEditorTypeName( caf::PdmUiDoubleValueEditor::uiEditorTypeName() );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathAicdParameters::~RimWellPathAicdParameters()
{
}

//--------------------------------------------------------------------------------------------------
/// Requires that all the required parameters are set and a proper value
//--------------------------------------------------------------------------------------------------
bool RimWellPathAicdParameters::isValid() const
{
    for ( const caf::PdmField<std::optional<double>>* stringField : optionalFieldsWithNoValidDefault() )
    {
        if ( !stringField->value().has_value() ) return false;
    }
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::setValue( AICDParameters parameter, double value )
{
    m_aicdParameterFields[parameter].setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathAicdParameters::isOpen() const
{
    return m_deviceOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<double, AICD_NUM_PARAMS> RimWellPathAicdParameters::doubleValues() const
{
    std::array<double, AICD_NUM_PARAMS> doubleValues;
    for ( int i = 0; i < (int)AICD_NUM_PARAMS; ++i )
    {
        std::optional<double> stringValue = m_aicdParameterFields[(AICDParameters)i].value();
        if ( stringValue.has_value() )
        {
            doubleValues[i] = stringValue.value();
        }
        else
        {
            doubleValues[i] = std::numeric_limits<double>::infinity();
        }
    }
    return doubleValues;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    bool readOnly = uiConfigName == QString( "InsideValve" );

    uiOrdering.add( &m_deviceOpen );
    m_deviceOpen.uiCapability()->setUiReadOnly( readOnly );
    for ( int i = 0; i < (int)AICD_NUM_REQ_PARAMS; ++i )
    {
        uiOrdering.add( &m_aicdParameterFields[(AICDParameters)i] );
        m_aicdParameterFields[(AICDParameters)i].uiCapability()->setUiReadOnly( readOnly );
    }

    caf::PdmUiGroup* additionalGroup = uiOrdering.addNewGroup( "Additional Parameters" );
    for ( int i = (int)AICD_NUM_REQ_PARAMS; i < (int)AICD_NUM_PARAMS; ++i )
    {
        additionalGroup->add( &m_aicdParameterFields[(AICDParameters)i] );
        m_aicdParameterFields[(AICDParameters)i].uiCapability()->setUiReadOnly( readOnly );
    }
    additionalGroup->setCollapsedByDefault();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::defineEditorAttribute( const caf::PdmFieldHandle* field, QString uiConfigName, caf::PdmUiEditorAttribute* attribute )
{
    const caf::PdmField<std::optional<double>>* optionalField  = dynamic_cast<const caf::PdmField<std::optional<double>>*>( field );
    caf::PdmUiLineEditorAttribute*              lineEditorAttr = dynamic_cast<caf::PdmUiLineEditorAttribute*>( attribute );
    if ( optionalField && lineEditorAttr )
    {
        if ( optionalFieldsWithNoValidDefault().count( optionalField ) )
        {
            lineEditorAttr->validator = new caf::PdmDoubleStringValidator( "" );
        }
        else
        {
            lineEditorAttr->validator = new caf::PdmDoubleStringValidator( "1*" );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<const caf::PdmField<std::optional<double>>*> RimWellPathAicdParameters::optionalFieldsWithNoValidDefault() const
{
    std::set<const caf::PdmField<std::optional<double>>*> fields;
    for ( int i = 0; i < (int)AICD_NUM_REQ_PARAMS; ++i )
    {
        fields.insert( &m_aicdParameterFields[(AICDParameters)i] );
    }
    return fields;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::setUnitLabels()
{
    if ( isMetric() )
    {
        m_aicdParameterFields[AICD_DENSITY_CALIB_FLUID].uiCapability()->setUiName( "Calibration Fluid Density (kg / m ^ 3)" );
        m_aicdParameterFields[AICD_MAX_FLOW_RATE].uiCapability()->setUiName( "Max Flow Rate for AICD Device(m ^ 3 / day)" );
    }
    else
    {
        m_aicdParameterFields[AICD_DENSITY_CALIB_FLUID].uiCapability()->setUiName( "Calibration Fluid Density (lb / ft ^3)" );
        m_aicdParameterFields[AICD_MAX_FLOW_RATE].uiCapability()->setUiName( "Max Flow Rate for AICD Device(ft ^ 3 / day)" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathAicdParameters::isMetric() const
{
    bool metric   = false;
    auto wellPath = firstAncestorOrThisOfType<RimWellPath>();
    if ( wellPath )
    {
        if ( wellPath->unitSystem() == RiaDefines::EclipseUnitSystem::UNITS_METRIC )
        {
            metric = true;
        }
    }
    return metric;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathAicdParameters::migrateFieldContent( QString& fieldContent, caf::PdmFieldHandle* fieldHandle )
{
    // Optional fields changed from QString to std::optional<double> in first release after v2025.04.4.
    // These fields used to have "1*" as default/empty value, but these are now converted to empty string.
    // Empty strings becomes an unset std::optional<double> in the xml parsing.

    // Check if the incoming field matches the new type.
    const caf::PdmField<std::optional<double>>* optionalField = dynamic_cast<const caf::PdmField<std::optional<double>>*>( fieldHandle );
    if ( !optionalField ) return;

    // Check if fieldContent is "1*" - if not, nothing to do.
    if ( fieldContent != "1*" ) return;

    // Find matching field in the array
    for ( const auto& field : m_aicdParameterFields )
    {
        if ( &field == optionalField )
        {
            // Found the matching field, clear the fieldContent
            fieldContent = QString();
            return;
        }
    }
}
