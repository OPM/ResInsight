/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
// 
//  ResInsight is free software: you can redistribute it and/or modify
//  it under the terms of the GNU General Public License as published by
//  the Free Software Foundation, either version 3 of the License, or
//  (at your option) any later version.
// +		m_addWellName	{m_fieldValue=true m_defaultFieldValue=true }	caf::PdmField<bool>

//  ResInsight is distributed in the hope that it will be useful, but WITHOUT ANY
//  WARRANTY; without even the implied warranty of MERCHANTABILITY or
//  FITNESS FOR A PARTICULAR PURPOSE.
// 
//  See the GNU General Public License at <http://www.gnu.org/licenses/gpl.html> 
//  for more details.
//
/////////////////////////////////////////////////////////////////////////////////

#include "RimProject.h"
#include "RimWellLogCurveNameConfig.h"
#include "Rim3dWellLogCurve.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimCurveNameConfig, "RimCurveNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCurveNameConfig::RimCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder /*= nullptr*/)
    : m_configHolder(configHolder)
{
    CAF_PDM_InitObject("Curve Name Generator", "", "", "");

    CAF_PDM_InitField(&m_isUsingAutoName, "IsUsingAutoName", true, "Generate Name Automatically", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_customName, "CustomCurveName", "Curve Name", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_autoName, "AutoCurveName", "Curve Name", "", "", "");
    m_autoName.registerGetMethod(this, &RimCurveNameConfig::autoName);
    m_autoName.registerSetMethod(this, &RimCurveNameConfig::setCustomName);
    m_autoName.xmlCapability()->disableIO();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCurveNameConfig::~RimCurveNameConfig()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimCurveNameConfig::isUsingAutoName() const
{
    return m_isUsingAutoName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCurveNameConfig::nameField()
{
    if (isUsingAutoName())
    {
        return &m_autoName;
    }
    return &m_customName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCurveNameConfig::name() const
{
    if (isUsingAutoName())
    {
        return m_autoName();
    }
    return m_customName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RimCurveNameConfig::createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name Configuration");
    nameGroup->add(&m_isUsingAutoName);
    return nameGroup;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCurveNameConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_customName)
    {
        m_isUsingAutoName = false;        
    }

    if (changedField == &m_isUsingAutoName && !isUsingAutoName())
    {
        m_customName = m_configHolder->createCurveAutoName();
    }

    updateAllSettings();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimCurveNameConfig::autoName() const
{
    return m_configHolder->createCurveAutoName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCurveNameConfig::setCustomName(const QString& name)
{
    m_isUsingAutoName = false;
    m_customName = name;
    updateAllSettings();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCurveNameConfig::updateAllSettings()
{
    m_isUsingAutoName.uiCapability()->updateConnectedEditors();
    m_autoName.uiCapability()->updateConnectedEditors();
    m_customName.uiCapability()->updateConnectedEditors();

    Rim3dWellLogCurve* curve;
    this->firstAncestorOrThisOfTypeAsserted(curve);
    curve->updateConnectedEditors();

}

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogExtractionCurveNameConfig, "RimWellLogExtractionCurveNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurveNameConfig::RimWellLogExtractionCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder)
    : RimCurveNameConfig(configHolder)
{
    CAF_PDM_InitObject("Well Log Extraction Curve Name Generator", "", "", "");

    CAF_PDM_InitField(&m_addCaseName, "AddCaseName", false, "Add Case Name To Auto Name", "", "", "");
    CAF_PDM_InitField(&m_addProperty, "AddProperty", true, "Add Property Type To Auto Name", "", "", "");
    CAF_PDM_InitField(&m_addWellName, "AddWellName", true, "Add Well Name To Auto Name", "", "", "");
    CAF_PDM_InitField(&m_addTimestep, "AddTimeStep", true, "Add Time Step To Auto Name", "", "", "");
    CAF_PDM_InitField(&m_addDate,     "AddDate",     true, "Add Date To Auto Name", "", "", "");

    m_customName = "Extraction Curve";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RimWellLogExtractionCurveNameConfig::createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* nameGroup = RimCurveNameConfig::createUiGroup(uiConfigName, uiOrdering);
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

    RimCurveNameConfig::updateAllSettings();
}

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogFileCurveNameConfig, "RimWellLogFileCurveNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurveNameConfig::RimWellLogFileCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder)
    : RimCurveNameConfig(configHolder)
{
    CAF_PDM_InitObject("Well Log File Curve Name Generator", "", "", "");
    m_customName = "Las Curve";
}

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimWellLogRftCurveNameConfig, "RimWellLogRftCurveNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurveNameConfig::RimWellLogRftCurveNameConfig(const RimCurveNameConfigHolderInterface* configHolder)
    : RimCurveNameConfig(configHolder)
{
    CAF_PDM_InitObject("Well Log Rft Curve Name Generator", "", "", "");
    m_customName = "Rft Curve";
}