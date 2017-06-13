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
#include "RiaDefines.h"


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

namespace caf
{
template<>
void caf::AppEnum< RimSummaryYAxisProperties::AxisTitlePositionType >::setUp()
{
    addItem(RimSummaryYAxisProperties::AXIS_TITLE_CENTER, "AXIS_TITLE_CENTER", "Center");
    addItem(RimSummaryYAxisProperties::AXIS_TITLE_END, "AXIS_TITLE_END", "At End");

    setDefault(RimSummaryYAxisProperties::AXIS_TITLE_CENTER);
}
}


CAF_PDM_SOURCE_INIT(RimSummaryYAxisProperties, "SummaryYAxisProperties");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryYAxisProperties::RimSummaryYAxisProperties()
{
    CAF_PDM_InitObject("Y-Axis Properties", ":/LeftAxis16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isAutoTitle, "AutoTitle", true, "Auto Title", "", "", "");
    CAF_PDM_InitFieldNoDefault(&customTitle, "CustomTitle", "Title", "", "", "");
    CAF_PDM_InitFieldNoDefault(&titlePositionEnum, "TitlePosition", "Title Position", "", "", "");
    CAF_PDM_InitField(&fontSize, "FontSize", 11, "Font Size", "", "", "");

    CAF_PDM_InitField(&visibleRangeMax, "VisibleRangeMax", RiaDefines::maximumDefaultValuePlot(), "Max", "", "", "");
    CAF_PDM_InitField(&visibleRangeMin, "VisibleRangeMin", RiaDefines::minimumDefaultValuePlot(), "Min", "", "", "");

    CAF_PDM_InitFieldNoDefault(&numberFormat, "NumberFormat", "Number Format", "", "", "");

    CAF_PDM_InitField(&isLogarithmicScaleEnabled, "LogarithmicScale", false, "Logarithmic Scale", "", "", "");

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
void RimSummaryYAxisProperties::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup& titleGroup = *(uiOrdering.addNewGroup("Axis Title"));
    titleGroup.add(&isAutoTitle);
    titleGroup.add(&customTitle);
    titleGroup.add(&titlePositionEnum);
    titleGroup.add(&fontSize);

    caf::PdmUiGroup& scaleGroup =  *(uiOrdering.addNewGroup("Axis Values"));
    scaleGroup.add(&isLogarithmicScaleEnabled);
    scaleGroup.add(&numberFormat);
    scaleGroup.add(&visibleRangeMin);
    scaleGroup.add(&visibleRangeMax);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryYAxisProperties::setNameAndAxis(const QString& name, QwtPlot::Axis axis)
{
    m_name = name;
    m_axis = axis;

    if (axis == QwtPlot::yRight) this->setUiIcon(QIcon(":/RightAxis16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QwtPlot::Axis RimSummaryYAxisProperties::qwtPlotAxisType() const
{
    return m_axis;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RimSummaryYAxisProperties::plotAxisType() const
{
    if (m_axis == QwtPlot::yRight) return RiaDefines::PLOT_AXIS_RIGHT;

    return RiaDefines::PLOT_AXIS_LEFT;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryYAxisProperties::isActive() const
{
    return m_isActive;
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
    if (rimSummaryPlot)
    {
        if (changedField == &visibleRangeMax)
        {
            if (visibleRangeMin > visibleRangeMax) visibleRangeMax = oldValue.toDouble();
        
            rimSummaryPlot->disableAutoZoom();
        }
        else if (changedField == &visibleRangeMin)
        {
            if (visibleRangeMin > visibleRangeMax) visibleRangeMin = oldValue.toDouble();
        
            rimSummaryPlot->disableAutoZoom();
        }

        if (changedField == &isLogarithmicScaleEnabled)
        {
            rimSummaryPlot->loadDataAndUpdate();
        }
        else
        {
            rimSummaryPlot->updateAxes();
        }
    }
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryYAxisProperties::objectToggleField()
{
    return &m_isActive;
}

