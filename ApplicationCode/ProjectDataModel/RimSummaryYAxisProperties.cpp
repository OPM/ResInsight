/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RimSummaryYAxisProperties.h"


CAF_PDM_SOURCE_INIT(RimSummaryYAxisProperties, "SummaryYAxisProperties");


namespace caf
{
    template<>
    void caf::AppEnum< RimSummaryYAxisProperties::NumberFormatType >::setUp()
    {
        addItem(RimSummaryYAxisProperties::NUMBER_FORMAT_AUTO,      "NUMBER_FORMAT_AUTO",       "Auto");
        addItem(RimSummaryYAxisProperties::NUMBER_FORMAT_DECIMAL,   "NUMBER_FORMAT_DECIMAL",    "Decimal");
        addItem(RimSummaryYAxisProperties::NUMBER_FORMAT_SCIENTIFIC,"NUMBER_FORMAT_SCIENTIFIC", "Scientific");

        setDefault(RimSummaryYAxisProperties::NUMBER_FORMAT_AUTO);
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryYAxisProperties::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryYAxisProperties::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if (&fontSize == fieldNeedingOptions)
    {
        std::vector<int> fontSizes;
        fontSizes.push_back(8);
        fontSizes.push_back(10);
        fontSizes.push_back(12);
        fontSizes.push_back(16);
        fontSizes.push_back(24);

        for (int value : fontSizes)
        {
            QString text = QString("%1").arg(value);
            options.push_back(caf::PdmOptionItemInfo(text, value));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryYAxisProperties::RimSummaryYAxisProperties()
{
    CAF_PDM_InitObject("Y-Axis Properties", ":/SummaryPlot16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isAutoTitle, "AutoTitle", true, "Auto Title", "", "", "");
    CAF_PDM_InitFieldNoDefault(&customTitle, "CustomTitle", "Title", "", "", "");
    CAF_PDM_InitField(&fontSize, "FontSize", 11, "Font Size", "", "", "");

    CAF_PDM_InitField(&isAutoScaleEnabled, "AutoScale", true, "Auto Scale", "", "", "");
    CAF_PDM_InitField(&visibleRangeMin, "VisibleRangeMin", m_minLogValueDefault, "Min", "", "", "");
    CAF_PDM_InitField(&visibleRangeMax, "VisibleRangeMax", m_maxLogValueDefault, "Max", "", "", "");

    CAF_PDM_InitFieldNoDefault(&numberFormat, "NumberFormat", "Number Format", "", "", "");

    CAF_PDM_InitField(&isLogarithmicScaleEnabled, "LogarithmicScale", false, "Logarithmic Scale", "", "", "");

    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::setName(const QString& name)
{
    m_name = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isAutoTitle ||
        changedField == &isAutoScaleEnabled)
    {
        updateOptionSensitivity();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::updateOptionSensitivity()
{
    customTitle.uiCapability()->setUiReadOnly(isAutoTitle);
    
    visibleRangeMin.uiCapability()->setUiReadOnly(isAutoScaleEnabled);
    visibleRangeMax.uiCapability()->setUiReadOnly(isAutoScaleEnabled);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::initAfterRead()
{
    updateOptionSensitivity();
}

