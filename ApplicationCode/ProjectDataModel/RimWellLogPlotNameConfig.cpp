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

#include "RimWellLogPlotNameConfig.h"

//==================================================================================================
///
///
//==================================================================================================

CAF_PDM_SOURCE_INIT( RimWellLogPlotNameConfig, "RimWellLogPlotNameConfig" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlotNameConfig::RimWellLogPlotNameConfig()
    : RimNameConfig( "Well Log Plot" )
{
    CAF_PDM_InitObject( "Well Log Plot Name Generator", "", "", "" );

    CAF_PDM_InitField( &m_addCaseName, "AddCaseName", false, "Show Case Name", "", "", "" );
    CAF_PDM_InitField( &m_addWellName, "AddWellName", false, "Show Well Name", "", "", "" );
    CAF_PDM_InitField( &m_addTimestep, "AddTimeStep", false, "Show Time Step", "", "", "" );
    CAF_PDM_InitField( &m_addAirGap, "AddAirGap", false, "Show Air Gap", "", "", "" );
    CAF_PDM_InitField( &m_addWaterDepth, "AddWaterDepth", false, "Show Water Depth", "", "", "" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotNameConfig::addCaseName() const
{
    return m_addCaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotNameConfig::addWellName() const
{
    return m_addWellName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotNameConfig::addTimeStep() const
{
    return m_addTimestep();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotNameConfig::addAirGap() const
{
    return m_addAirGap();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimWellLogPlotNameConfig::addWaterDepth() const
{
    return m_addWaterDepth();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotNameConfig::setAutoNameTags( bool addCaseName,
                                                bool addWellName,
                                                bool addTimeStep,
                                                bool addAirGap,
                                                bool addWaterDepth )
{
    m_addCaseName   = addCaseName;
    m_addWellName   = addWellName;
    m_addTimestep   = addTimeStep;
    m_addAirGap     = addAirGap;
    m_addWaterDepth = addWaterDepth;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotNameConfig::setFieldVisibility( bool caseNameVisible,
                                                   bool wellNameVisible,
                                                   bool timeStepVisible,
                                                   bool airGapVisible,
                                                   bool waterDepthVisible )
{
    m_addCaseName.uiCapability()->setUiHidden( !caseNameVisible );
    m_addWellName.uiCapability()->setUiHidden( !wellNameVisible );
    m_addTimestep.uiCapability()->setUiHidden( !timeStepVisible );
    m_addAirGap.uiCapability()->setUiHidden( !airGapVisible );
    m_addWaterDepth.uiCapability()->setUiHidden( !waterDepthVisible );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotNameConfig::doEnableAllAutoNameTags( bool enable )
{
    setAutoNameTags( enable, enable, enable, enable, enable );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogPlotNameConfig::defineUiOrdering( QString uiConfigName, caf::PdmUiOrdering& uiOrdering )
{
    RimNameConfig::defineUiOrdering( uiConfigName, uiOrdering );

    uiOrdering.add( &m_addCaseName );
    uiOrdering.add( &m_addWellName );
    uiOrdering.add( &m_addTimestep );
    uiOrdering.add( &m_addAirGap );
    uiOrdering.add( &m_addWaterDepth );
}
