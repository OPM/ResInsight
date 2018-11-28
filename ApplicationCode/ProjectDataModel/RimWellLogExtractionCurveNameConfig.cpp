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

#include "RimWellLogExtractionCurveNameConfig.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogExtractionCurveNameConfig, "RimWellLogExtractionCurveNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurveNameConfig::RimWellLogExtractionCurveNameConfig(const RimNameConfigHolderInterface* configHolder)
    : RimNameConfig(configHolder)
{
    CAF_PDM_InitObject("Well Log Extraction Curve Name Generator", "", "", "");

    CAF_PDM_InitField(&m_addCaseName, "AddCaseName", true, "Add Case Name", "", "", "");
    CAF_PDM_InitField(&m_addProperty, "AddProperty", true, "Add Property Type", "", "", "");
    CAF_PDM_InitField(&m_addWellName, "AddWellName", true, "Add Well Name", "", "", "");
    CAF_PDM_InitField(&m_addTimestep, "AddTimeStep", true, "Add Time Step", "", "", "");
    CAF_PDM_InitField(&m_addDate,     "AddDate",     true, "Add Date", "", "", "");

    m_customName = "Log Extraction";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurveNameConfig::addCaseName() const
{
    return m_addCaseName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurveNameConfig::addProperty() const
{
    return m_addProperty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurveNameConfig::addWellName() const
{
    return m_addWellName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurveNameConfig::addTimeStep() const
{
    return m_addTimestep();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellLogExtractionCurveNameConfig::addDate() const
{
    return m_addDate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurveNameConfig::enableAllAutoNameTags(bool enable)
{
    m_addCaseName = enable;
    m_addProperty = enable;
    m_addWellName = enable;
    m_addTimestep = enable;
    m_addDate     = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimWellLogExtractionCurveNameConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimNameConfig::defineUiOrdering(uiConfigName, uiOrdering);
    uiOrdering.add(&m_addCaseName);
    uiOrdering.add(&m_addProperty);
    uiOrdering.add(&m_addWellName);
    uiOrdering.add(&m_addTimestep);
    uiOrdering.add(&m_addDate);
}
