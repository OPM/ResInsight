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
#include "RimNameConfig.h"
#include "Rim3dWellLogCurve.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimNameConfig, "RimCurveNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimNameConfig::RimNameConfig(const RimNameConfigHolderInterface* configHolder /*= nullptr*/)
    : m_configHolder(configHolder)
{
    CAF_PDM_InitObject("Curve Name Generator", "", "", "");

    CAF_PDM_InitField(&m_isUsingAutoName_OBSOLETE, "IsUsingAutoName", true, "Add Automatic Name Tags", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_customName, "CustomCurveName", "Custom Name Part", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_autoName, "AutoCurveName", "Full Curve Name", "", "", "");
    m_isUsingAutoName_OBSOLETE.xmlCapability()->setIOWritable(false);
    m_autoName.registerGetMethod(this, &RimNameConfig::autoName);
    m_autoName.xmlCapability()->disableIO();
    m_autoName.uiCapability()->setUiReadOnly(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimNameConfig::~RimNameConfig()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RimNameConfig::customName() const
{
    return m_customName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimNameConfig::nameField()
{
    return &m_autoName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimNameConfig::name() const
{
    return m_autoName();    
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::uiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    defineUiOrdering(uiConfigName, uiOrdering);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.add(&m_customName);
    uiOrdering.add(&m_autoName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimNameConfig::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    updateAllSettings();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimNameConfig::autoName() const
{
    return m_configHolder->createAutoName();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimNameConfig::setCustomName(const QString& name)
{
    m_customName = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimNameConfig::updateAllSettings()
{
    m_autoName.uiCapability()->updateConnectedEditors();
    m_customName.uiCapability()->updateConnectedEditors();

    RimNameConfigHolderInterface* holder;
    this->firstAncestorOrThisOfTypeAsserted(holder);
    holder->updateHolder();
    caf::PdmObject* pdmObject = dynamic_cast<caf::PdmObject*>(holder);
    if (pdmObject)
    {
        pdmObject->updateConnectedEditors();        
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimNameConfig::initAfterRead()
{
    // Now we just switch them all individually.
    if (!m_isUsingAutoName_OBSOLETE())
    {
        enableAllAutoNameTags(false);
    }

    updateAllSettings();
}
