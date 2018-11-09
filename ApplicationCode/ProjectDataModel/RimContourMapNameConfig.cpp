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

#include "RimContourMapNameConfig.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimContourMapNameConfig, "RimContourMapNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimContourMapNameConfig::RimContourMapNameConfig(const RimNameConfigHolderInterface* configHolder)
    : RimNameConfig(configHolder)
{
    CAF_PDM_InitObject("Contour Map Name Generator", "", "", "");

    CAF_PDM_InitField(&m_addCaseName, "AddCaseName", true, "Add Case Name", "", "", "");
    CAF_PDM_InitField(&m_addAggregationType, "AddAggregationType", true, "Add Aggregation Type", "", "", "");
    CAF_PDM_InitField(&m_addProperty, "AddProperty", true, "Add Property Type", "", "", "");
    CAF_PDM_InitField(&m_addSampleSpacing, "AddSampleSpacing", false, "Add Sample Spacing", "", "", "");

    m_customName = "Contour Map";
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmUiGroup* RimContourMapNameConfig::createUiGroup(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* nameGroup = RimNameConfig::createUiGroup(uiConfigName, uiOrdering);
    nameGroup->add(&m_addCaseName);
    nameGroup->add(&m_addAggregationType);
    nameGroup->add(&m_addProperty);
    nameGroup->add(&m_addSampleSpacing);
    return nameGroup;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapNameConfig::addCaseName() const
{
    return m_addCaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapNameConfig::addAggregationType() const
{
    return m_addAggregationType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapNameConfig::addProperty() const
{
    return m_addProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimContourMapNameConfig::addSampleSpacing() const
{
    return m_addSampleSpacing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimContourMapNameConfig::enableAllAutoNameTags(bool enable)
{
    m_addCaseName        = enable;
    m_addAggregationType = enable;
    m_addProperty        = enable;
    m_addSampleSpacing   = enable;
}
