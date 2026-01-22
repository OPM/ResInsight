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

    CAF_PDM_InitScriptableFieldNoDefault( &m_strength, "Strength", "Strength of the SICD device" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_length, "Length", "Length of the SICD device" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_calibrationDensity, "CalibrationDensity", "Calibration Fluid Density" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_calibrationViscosity, "CalibrationViscosity", "Calibration Fluid Viscosity" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_emlCrt, "EmlCrt", "Local Water in Liquid Fraction (EMLCRT)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_emlTrans, "EmlTrans", "Width of Transition Zone (EMLTRNS)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_emlMax, "EmlMax", "Max Emulsion Viscosity to Cont Phase Viscosity (EMLMAX)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_scaleFactorType, "ScaleFactorType", "Scale Factor Method (NSCALFAC)" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_maxCalibRate, "MaxCalibRate", "Max Surface Flow Rate (CALRATE)" );
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
    return true;
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
}
