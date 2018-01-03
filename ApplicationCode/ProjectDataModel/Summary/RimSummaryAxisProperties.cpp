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

#include "RimSummaryAxisProperties.h"

#include "RiaDefines.h"
#include "RimSummaryPlot.h"

#include "cafPdmUiSliderEditor.h"

#include <cmath>

// clang-format off
namespace caf
{
template<>
void caf::AppEnum<RimSummaryAxisProperties::NumberFormatType>::setUp()
{
    addItem(RimSummaryAxisProperties::NUMBER_FORMAT_AUTO,       "NUMBER_FORMAT_AUTO",       "Auto");
    addItem(RimSummaryAxisProperties::NUMBER_FORMAT_DECIMAL,    "NUMBER_FORMAT_DECIMAL",    "Decimal");
    addItem(RimSummaryAxisProperties::NUMBER_FORMAT_SCIENTIFIC, "NUMBER_FORMAT_SCIENTIFIC", "Scientific");

    setDefault(RimSummaryAxisProperties::NUMBER_FORMAT_AUTO);
}

template<>
void caf::AppEnum<RimSummaryAxisProperties::AxisTitlePositionType>::setUp()
{
    addItem(RimSummaryAxisProperties::AXIS_TITLE_CENTER, "AXIS_TITLE_CENTER",   "Center");
    addItem(RimSummaryAxisProperties::AXIS_TITLE_END,    "AXIS_TITLE_END",      "At End");

    setDefault(RimSummaryAxisProperties::AXIS_TITLE_CENTER);
}
} // namespace caf

CAF_PDM_SOURCE_INIT(RimSummaryAxisProperties, "SummaryYAxisProperties");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryAxisProperties::RimSummaryAxisProperties()
{
    CAF_PDM_InitObject("Y-Axis Properties", ":/LeftAxis16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_name, "Name", "Name", "", "", "");
    m_name.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&isAutoTitle, "AutoTitle", true, "Auto Title", "", "", "");
    
    CAF_PDM_InitField(&m_displayLongName,   "DisplayLongName",  true,   "   Names", "", "", "");
    CAF_PDM_InitField(&m_displayShortName,  "DisplayShortName", false,  "   Acronyms", "", "", "");
    CAF_PDM_InitField(&m_displayUnitText,   "DisplayUnitText",  true,   "   Units", "", "", "");

    CAF_PDM_InitFieldNoDefault(&customTitle,        "CustomTitle",      "Title", "", "", "");
    CAF_PDM_InitFieldNoDefault(&titlePositionEnum,  "TitlePosition",    "Title Position", "", "", "");
    CAF_PDM_InitField(&titleFontSize,               "FontSize", 11,     "Font Size", "", "", "");

    CAF_PDM_InitField(&visibleRangeMax, "VisibleRangeMax", RiaDefines::maximumDefaultValuePlot(), "Max", "", "", "");
    CAF_PDM_InitField(&visibleRangeMin, "VisibleRangeMin", RiaDefines::minimumDefaultValuePlot(), "Min", "", "", "");

    CAF_PDM_InitFieldNoDefault(&numberFormat,   "NumberFormat",         "Number Format", "", "", "");
    CAF_PDM_InitField(&numberOfDecimals,        "Decimals", 2,          "Number of Decimals", "", "", "");
    CAF_PDM_InitField(&scaleFactor,             "ScaleFactor", 1.0,     "Scale Factor", "", "", "");
    CAF_PDM_InitField(&valuesFontSize,          "ValuesFontSize", 11,   "Font Size", "", "", "");

    numberOfDecimals.uiCapability()->setUiEditorTypeName(caf::PdmUiSliderEditor::uiEditorTypeName());

    CAF_PDM_InitField(&isLogarithmicScaleEnabled, "LogarithmicScale", false, "Logarithmic Scale", "", "", "");

    updateOptionSensitivity();
}

// clang-format on

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryAxisProperties::userDescriptionField()
{
    return &m_name;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryAxisProperties::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions,
                                                                              bool*                      useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if (&titleFontSize == fieldNeedingOptions || &valuesFontSize == fieldNeedingOptions)
    {
        std::vector<int> fontSizes;
        fontSizes.push_back(8);
        fontSizes.push_back(9);
        fontSizes.push_back(10);
        fontSizes.push_back(11);
        fontSizes.push_back(12);
        fontSizes.push_back(14);
        fontSizes.push_back(16);
        fontSizes.push_back(18);
        fontSizes.push_back(24);

        for (int value : fontSizes)
        {
            QString text = QString("%1").arg(value);
            options.push_back(caf::PdmOptionItemInfo(text, value));
        }
    }
    else if (fieldNeedingOptions == &scaleFactor)
    {
        for (int exp = -12; exp <= 12; exp += 3)
        {
            QString uiText = exp == 0 ? "1" : QString("10 ^ %1").arg(exp);
            double  value  = std::pow(10, exp);

            options.push_back(caf::PdmOptionItemInfo(uiText, value));
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAxisProperties::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiGroup* titleTextGroup = uiOrdering.addNewGroup("Title Text");

        titleTextGroup->add(&isAutoTitle);

        if (isAutoTitle())
        {
            titleTextGroup->add(&m_displayLongName);
            titleTextGroup->add(&m_displayShortName);
            titleTextGroup->add(&m_displayUnitText);

            customTitle.uiCapability()->setUiReadOnly(true);
        }
        else
        {
            titleTextGroup->add(&customTitle);
            customTitle.uiCapability()->setUiReadOnly(false);
        }
    }

    {
        caf::PdmUiGroup* titleGroup = uiOrdering.addNewGroup("Title Layout");
        titleGroup->add(&titlePositionEnum);
        titleGroup->add(&titleFontSize);
    }

    caf::PdmUiGroup& scaleGroup = *(uiOrdering.addNewGroup("Axis Values"));
    scaleGroup.add(&isLogarithmicScaleEnabled);
    scaleGroup.add(&numberFormat);

    if (numberFormat() != NUMBER_FORMAT_AUTO)
    {
        scaleGroup.add(&numberOfDecimals);
        scaleGroup.add(&scaleFactor);
    }

    scaleGroup.add(&visibleRangeMin);
    scaleGroup.add(&visibleRangeMax);
    scaleGroup.add(&valuesFontSize);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAxisProperties::setNameAndAxis(const QString& name, QwtPlot::Axis axis)
{
    m_name = name;
    m_axis = axis;

    if (axis == QwtPlot::yRight) this->setUiIcon(QIcon(":/RightAxis16x16.png"));
    if (axis == QwtPlot::xBottom) this->setUiIcon(QIcon(":/BottomAxis16x16.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QwtPlot::Axis RimSummaryAxisProperties::qwtPlotAxisType() const
{
    return m_axis;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RimSummaryAxisProperties::plotAxisType() const
{
    if (m_axis == QwtPlot::yRight) return RiaDefines::PLOT_AXIS_RIGHT;
    if (m_axis == QwtPlot::xBottom) return RiaDefines::PLOT_AXIS_BOTTOM;

    return RiaDefines::PLOT_AXIS_LEFT;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAxisProperties::useAutoTitle() const
{
    return isAutoTitle();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAxisProperties::showDescription() const
{
    return m_displayLongName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAxisProperties::showAcronym() const
{
    return m_displayShortName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAxisProperties::showUnitText() const
{
    return m_displayUnitText();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimSummaryAxisProperties::isActive() const
{
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAxisProperties::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue,
                                                const QVariant& newValue)
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
void RimSummaryAxisProperties::updateOptionSensitivity()
{
    customTitle.uiCapability()->setUiReadOnly(isAutoTitle);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimSummaryAxisProperties::initAfterRead()
{
    updateOptionSensitivity();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimSummaryAxisProperties::objectToggleField()
{
    return &m_isActive;
}
