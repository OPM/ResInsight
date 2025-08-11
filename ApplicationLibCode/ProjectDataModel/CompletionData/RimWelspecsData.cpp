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

#include "RimWelspecsData.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"

CAF_PDM_SOURCE_INIT( RimWelspecsData, "Welspecs" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWelspecsData::RimWelspecsData()
{
    CAF_PDM_InitScriptableObject( "Welspecs", ":/Well.png" );

    CAF_PDM_InitScriptableFieldNoDefault( &m_wellname, "WellName", "Well Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_groupname, "GroupName", "Group Name" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_I, "WellI", "I" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_J, "WellJ", "J" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_bhpDepth, "BhpDepth", "BHP Depth" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_phase, "Phase", "Phase" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_drainageRadius, "DrainageRadius", "Drainage Radius" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_inflowEquation, "InflowEquation", "Inflow Equation" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_autoShutIn, "AutoShutIn", "Auto Shut In" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_crossFlow, "CrossFlow", "Cross Flow" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_pvtTableNum, "PvtTableNum", "PVT Table Number" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_hydrostaticDensCalc, "HydrostaticDensCalc", "Hydrostatic Density Calculation" );
    CAF_PDM_InitScriptableFieldNoDefault( &m_fipRegion, "FipRegion", "FIP Region" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWelspecsData::~RimWelspecsData()
{
}
