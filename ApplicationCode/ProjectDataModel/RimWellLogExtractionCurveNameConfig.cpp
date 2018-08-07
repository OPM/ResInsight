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

    CAF_PDM_InitField(&m_addCaseName, "AddCaseName", false, "Add Case Name", "", "", "");
    CAF_PDM_InitField(&m_addProperty, "AddProperty", true, "Add Property Type", "", "", "");
    CAF_PDM_InitField(&m_addWellName, "AddWellName", true, "Add Well Name", "", "", "");
    CAF_PDM_InitField(&m_addTimestep, "AddTimeStep", true, "Add Time Step", "", "", "");
    CAF_PDM_InitField(&m_addDate,     "AddDate",     true, "Add Date", "", "", "");

    m_customName = "Extraction Curve";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RimWellLogExtractionCurveNameConfig::createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* nameGroup = RimNameConfig::createUiGroup(uiConfigName, uiOrdering);
    nameGroup->add(&m_addCaseName);
    nameGroup->add(&m_addProperty);
    nameGroup->add(&m_addWellName);
    nameGroup->add(&m_addTimestep);
    nameGroup->add(&m_addDate);
    return nameGroup;
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
void RimWellLogExtractionCurveNameConfig::updateAllSettings()
{
    m_addCaseName.uiCapability()->setUiReadOnly(!isUsingAutoName());
    m_addProperty.uiCapability()->setUiReadOnly(!isUsingAutoName());
    m_addWellName.uiCapability()->setUiReadOnly(!isUsingAutoName());
    m_addTimestep.uiCapability()->setUiReadOnly(!isUsingAutoName());
    m_addDate.uiCapability()->setUiReadOnly(!isUsingAutoName());

    RimNameConfig::updateAllSettings();
}


