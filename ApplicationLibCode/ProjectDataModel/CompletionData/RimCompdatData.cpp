/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "RimCompdatData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimCompdatData, "Compdat" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCompdatData::RimCompdatData()
{
    CAF_PDM_InitScriptableObject( "Compdat", ":/Well.png" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_wellname, "WellName", "Well Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_I, "GridI", "Grid I" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_J, "GridJ", "Grid J" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_upperK, "UpperK", "Upper K" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_lowerK, "LowerK", "Lower K" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_openShutFlag, "OpenShutFlag", "Open/Shut Flag" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_satTableNum, "SatTableNum", "Saturation Table Number" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_transmissibility, "Transmissibility", "Transmissibility" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_diameter, "Diameter", "Diameter" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_kh, "Kh", "Kh Factor" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_skinFactor, "SkinFactor", "Skin Factor" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_dFactor, "DFactor", "D Factor" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_direction, "Direction", "Direction" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_startMD, "StartMd", "Start MD" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_endMD, "EndMd", "End MD" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCompdatData::~RimCompdatData()
{
}
