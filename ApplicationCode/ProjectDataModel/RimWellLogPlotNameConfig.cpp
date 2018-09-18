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

CAF_PDM_SOURCE_INIT(RimWellLogPlotNameConfig, "RimWellLogPlotNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogPlotNameConfig::RimWellLogPlotNameConfig(const RimNameConfigHolderInterface* configHolder)
    : RimNameConfig(configHolder)
{
    CAF_PDM_InitObject("Well Log Plot Name Generator", "", "", "");

    CAF_PDM_InitField(&m_addCaseName,   "AddCaseName",   false, "Add Case Name",   "", "", "");
    CAF_PDM_InitField(&m_addWellName,   "AddWellName",   false, "Add Well Name",   "", "", "");
    CAF_PDM_InitField(&m_addTimestep,   "AddTimeStep",   false, "Add Time Step",   "", "", "");
    CAF_PDM_InitField(&m_addAirGap,     "AddAirGap",     false, "Add Air Gap",     "", "", "");
    CAF_PDM_InitField(&m_addWaterDepth, "AddWaterDepth", false, "Add Water Depth", "", "", "");

    m_customName = "Well Log Plot";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RimWellLogPlotNameConfig::createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* nameGroup = RimNameConfig::createUiGroup(uiConfigName, uiOrdering);
    nameGroup->add(&m_addCaseName);
    nameGroup->add(&m_addWellName);
    nameGroup->add(&m_addTimestep);
    nameGroup->add(&m_addAirGap);
    nameGroup->add(&m_addWaterDepth);
    return nameGroup;
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
void RimWellLogPlotNameConfig::enableAllAutoNameTags(bool enable)
{
    m_addCaseName   = enable;
    m_addWellName   = enable;
    m_addTimestep   = enable;
    m_addAirGap     = enable;
    m_addWaterDepth = enable;
}

