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

#include "RimSummaryPlot.h"
#include "RimDefines.h"


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

CAF_PDM_SOURCE_INIT(RimSummaryYAxisProperties, "SummaryYAxisProperties");

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

    CAF_PDM_InitField(&visibleRangeMax, "VisibleRangeMax", RimDefines::maximumDefaultValuePlot(), "Max", "", "", "");
    CAF_PDM_InitField(&visibleRangeMin, "VisibleRangeMin", RimDefines::minimumDefaultValuePlot(), "Min", "", "", "");

    CAF_PDM_InitFieldNoDefault(&numberFormat, "NumberFormat", "Number Format", "", "", "");

    CAF_PDM_InitField(&isLogarithmicScaleEnabled, "LogarithmicScale", false, "Logarithmic Scale", "", "", "");
    isLogarithmicScaleEnabled.uiCapability()->setUiHidden(true);

    updateOptionSensitivity();
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
void RimSummaryYAxisProperties::setNameAndAxis(const QString& name, QwtPlot::Axis axis)
{
    m_name = name;
    m_axis = axis;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtPlot::Axis RimSummaryYAxisProperties::axis() const
{
    return m_axis;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &isAutoTitle)
    {
        updateOptionSensitivity();
    }

    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType(rimSummaryPlot);
    rimSummaryPlot->updateLeftAndRightYAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::updateOptionSensitivity()
{
    customTitle.uiCapability()->setUiReadOnly(isAutoTitle);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::initAfterRead()
{
    updateOptionSensitivity();
}

