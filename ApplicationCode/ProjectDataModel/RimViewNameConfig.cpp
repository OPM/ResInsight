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

#include "RimViewNameConfig.h"

//==================================================================================================
///  
///  
//==================================================================================================

CAF_PDM_SOURCE_INIT(RimViewNameConfig, "RimViewNameConfig");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimViewNameConfig::RimViewNameConfig(const RimNameConfigHolderInterface* configHolder)
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
bool RimViewNameConfig::addCaseName() const
{
    return m_addCaseName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewNameConfig::addAggregationType() const
{
    return m_addAggregationType();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewNameConfig::addProperty() const
{
    return m_addProperty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimViewNameConfig::addSampleSpacing() const
{
    return m_addSampleSpacing();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::enableAllAutoNameTags(bool enable)
{
    m_addCaseName        = enable;
    m_addAggregationType = enable;
    m_addProperty        = enable;
    m_addSampleSpacing   = enable;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimViewNameConfig::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimNameConfig::defineUiOrdering(uiConfigName, uiOrdering);
    uiOrdering.add(&m_addCaseName);
    uiOrdering.add(&m_addAggregationType);
    uiOrdering.add(&m_addProperty);
    uiOrdering.add(&m_addSampleSpacing);
}
