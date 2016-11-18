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

#include "RimSummaryTimeAxisProperties.h"

#include "RimSummaryPlot.h"

#include "cafPdmUiLineEditor.h"

#include "qwt_date.h"


namespace caf
{
template<>
void caf::AppEnum< RimSummaryTimeAxisProperties::AxisTitlePositionType >::setUp()
{
    addItem(RimSummaryTimeAxisProperties::AXIS_TITLE_CENTER, "AXIS_TITLE_CENTER", "Center");
    addItem(RimSummaryTimeAxisProperties::AXIS_TITLE_END, "AXIS_TITLE_END", "At End");

    setDefault(RimSummaryTimeAxisProperties::AXIS_TITLE_CENTER);
}
}



CAF_PDM_SOURCE_INIT(RimSummaryTimeAxisProperties, "SummaryTimeAxisProperties");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryTimeAxisProperties::RimSummaryTimeAxisProperties()
{
    CAF_PDM_InitObject("Time Axis", ":/BottomAxis16x16.png", "", "");

    CAF_PDM_InitField(&m_isActive, "Active", true, "Active", "", "", "");
    m_isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&showTitle, "ShowTitle", false, "Show Title", "", "", "");
    CAF_PDM_InitField(&title, "Title", QString("Time"), "Title", "", "", "");
    CAF_PDM_InitFieldNoDefault(&titlePositionEnum, "TitlePosition", "Title Position", "", "", "");

    CAF_PDM_InitField(&fontSize, "FontSize", 11, "Font Size", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_visibleRangeMax, "VisibleRangeMax", "Max", "", "", "");
    m_visibleRangeMax.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_visibleRangeMin, "VisibleRangeMin", "Min", "", "", "");
    m_visibleRangeMin.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::visibleRangeMin() const
{
    return QwtDate::toDouble(m_visibleRangeMin());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::visibleRangeMax() const
{
    return QwtDate::toDouble(m_visibleRangeMax());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleRangeMin(double value)
{
    m_visibleRangeMin = QwtDate::toDateTime(value);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleRangeMax(double value)
{
    m_visibleRangeMax = QwtDate::toDateTime(value);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryTimeAxisProperties::isActive() const
{
    return m_isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimSummaryTimeAxisProperties::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    if (&fontSize == fieldNeedingOptions)
    {
        std::vector<int> fontSizes;
        fontSizes.push_back(8);
        fontSizes.push_back(10);
        fontSizes.push_back(11);
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
caf::PdmFieldHandle* RimSummaryTimeAxisProperties::objectToggleField()
{
    return &m_isActive;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType(rimSummaryPlot);

    if (changedField == &m_visibleRangeMax)
    {
        QDateTime test = newValue.toDateTime();
        if (!test.isValid())
        {
            m_visibleRangeMax = oldValue.toDateTime();
        }

        rimSummaryPlot->disableAutoZoom();
    }
    else if (changedField == &m_visibleRangeMin)
    {
        QDateTime test = newValue.toDateTime();
        if (!test.isValid())
        {
            m_visibleRangeMin = oldValue.toDateTime();
        }

        rimSummaryPlot->disableAutoZoom();
    }

    rimSummaryPlot->updateAxes();
}

