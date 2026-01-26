/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026     Equinor ASA
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
#include "RimWellPathSicdParameters.h"

#include "RimPerforationInterval.h"
#include "RimWellPath.h"
#include "RimWellPathValve.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmUiGroup.h"
#include "cafPdmUiLineEditor.h"

CAF_PDM_SOURCE_INIT( RimWellPathSicdParameters, "WellPathSicdParameters" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathSicdParameters::RimWellPathSicdParameters()
{
    CAF_PDM_InitScriptableObject( "RimWellPathSicdParameters" );

    CAF_PDM_InitScriptableField( &m_sicdParameterFields[SICD_STRENGTH], "Strength", std::optional<double>( 1.0 ), "Strength of the SICD device" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_sicdParameterFields[SICD_CALIBRATION_DENSITY], "CalibrationDensity", "Calibration Fluid Density" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_sicdParameterFields[SICD_CALIBRATION_VISCOSITY],
                                          "CalibrationViscosity",
                                          "Calibration Fluid Viscosity" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_sicdParameterFields[SICD_EML_CRT], "EmlCrt", "Local Water in Liquid Fraction (EMLCRT)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_sicdParameterFields[SICD_EML_TRANS], "EmlTrans", "Width of Transition Zone (EMLTRNS)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_sicdParameterFields[SICD_EML_MAX],
                                          "EmlMax",
                                          "Max Emulsion Viscosity to Cont Phase Viscosity (EMLMAX)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_sicdParameterFields[SICD_MAX_CALIB_RATE], "MaxCalibRate", "Max Surface Flow Rate (CALRATE)" );
    CAF_PDM_InitScriptableField( &m_deviceOpen, "DeviceOpen", true, "Device Open?" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPathSicdParameters::~RimWellPathSicdParameters()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathSicdParameters::isValid() const
{
    return m_sicdParameterFields[SICD_STRENGTH].value().has_value();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellPathSicdParameters::isOpen() const
{
    return m_deviceOpen;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathSicdParameters::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    bool readOnly = uiConfigName == QString( "InsideValve" );

    uiOrdering.add( &m_deviceOpen );
    m_deviceOpen.uiCapability()->setUiReadOnly( readOnly );

    for ( int i = 0; i < (int)SICD_NUM_PARAMS; i++ )
    {
        uiOrdering.add( &m_sicdParameterFields[(SICDParameters)i] );
        m_sicdParameterFields[(SICDParameters)i].uiCapability()->setUiReadOnly( readOnly );
    }

    uiOrdering.skipRemainingFields( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellPathSicdParameters::setValue( SICDParameters parameter, double value )
{
    m_sicdParameterFields[parameter].setValue( value );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::array<double, SICD_NUM_PARAMS> RimWellPathSicdParameters::doubleValues() const
{
    std::array<double, SICD_NUM_PARAMS> doubleValues;
    for ( int i = 0; i < (int)SICD_NUM_PARAMS; ++i )
    {
        std::optional<double> dValue = m_sicdParameterFields[(SICDParameters)i].value();
        if ( dValue.has_value() )
        {
            doubleValues[i] = dValue.value();
        }
        else
        {
            doubleValues[i] = std::numeric_limits<double>::infinity();
        }
    }
    return doubleValues;
}
