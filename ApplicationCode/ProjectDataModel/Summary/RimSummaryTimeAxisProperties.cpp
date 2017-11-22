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
#include "cvfAssert.h"


namespace caf
{
template<>
void caf::AppEnum< RimSummaryTimeAxisProperties::AxisTitlePositionType >::setUp()
{
    addItem(RimSummaryTimeAxisProperties::AXIS_TITLE_CENTER, "AXIS_TITLE_CENTER", "Center");
    addItem(RimSummaryTimeAxisProperties::AXIS_TITLE_END, "AXIS_TITLE_END", "At End");

    setDefault(RimSummaryTimeAxisProperties::AXIS_TITLE_CENTER);
}

template<>
void caf::AppEnum< RimSummaryTimeAxisProperties::TimeModeType >::setUp()
{
    addItem(RimSummaryTimeAxisProperties::DATE, "DATE", "Date");
    addItem(RimSummaryTimeAxisProperties::TIME_FROM_SIMULATION_START, "TIME_FROM_SIMULATION_START", "Time From Simulation Start");

    setDefault(RimSummaryTimeAxisProperties::DATE);
}

template<>
void caf::AppEnum< RimSummaryTimeAxisProperties::TimeUnitType >::setUp()
{
    addItem(RimSummaryTimeAxisProperties::SECONDS, "SECONDS", "Seconds");
    addItem(RimSummaryTimeAxisProperties::MINUTES, "MINUTES", "Minutes");
    addItem(RimSummaryTimeAxisProperties::HOURS, "HOURS", "Hours");
    addItem(RimSummaryTimeAxisProperties::DAYS,  "DAYS ", "Days");
    addItem(RimSummaryTimeAxisProperties::YEARS, "YEARS", "Years");

    setDefault(RimSummaryTimeAxisProperties::YEARS);
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

    CAF_PDM_InitFieldNoDefault(&m_timeMode, "TimeMode", "Time Mode", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_timeUnit, "TimeUnit", "Time Unit", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_visibleDateRangeMax, "VisibleRangeMax", "Max", "", "", "");
    m_visibleDateRangeMax.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_visibleDateRangeMin, "VisibleRangeMin", "Min", "", "", "");
    m_visibleDateRangeMin.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_visibleTimeRangeMax, "VisibleTimeModeRangeMax", "Max", "", "", "");
    m_visibleDateRangeMax.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_visibleTimeRangeMin, "VisibleTimeModeRangeMin", "Min", "", "", "");
    m_visibleDateRangeMin.uiCapability()->setUiEditorTypeName(caf::PdmUiLineEditor::uiEditorTypeName());

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::visibleRangeMin() const
{
    if ( m_timeMode() == DATE )
        return QwtDate::toDouble(m_visibleDateRangeMin());
    else
        return m_visibleTimeRangeMin();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::visibleRangeMax() const
{
    if ( m_timeMode() == DATE )
        return QwtDate::toDouble(m_visibleDateRangeMax());
    else
        return m_visibleTimeRangeMax();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleRangeMin(double value)
{
    if ( m_timeMode() == DATE )
    {
        m_visibleDateRangeMin = QwtDate::toDateTime(value);
        m_visibleTimeRangeMin = fromDateToDisplayTime(m_visibleDateRangeMin());
    }
    else
    {
        m_visibleTimeRangeMin = value;
        m_visibleDateRangeMin = fromDisplayTimeToDate(value);
    }
    auto s = m_visibleDateRangeMin().toString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::setVisibleRangeMax(double value)
{
    if ( m_timeMode() == DATE )
    {
        m_visibleDateRangeMax = QwtDate::toDateTime(value);
        m_visibleTimeRangeMax = fromDateToDisplayTime(m_visibleDateRangeMax());

    }
    else
    {
        m_visibleTimeRangeMax = value;
        m_visibleDateRangeMax = fromDisplayTimeToDate(value);
    }
    auto s = m_visibleDateRangeMax().toString();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::updateTimeVisibleRange()
{
    m_visibleTimeRangeMax = fromDateToDisplayTime(m_visibleDateRangeMax());
    m_visibleTimeRangeMin = fromDateToDisplayTime(m_visibleDateRangeMin());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::updateDateVisibleRange()
{
    m_visibleDateRangeMin = fromDisplayTimeToDate(m_visibleTimeRangeMin());
    m_visibleDateRangeMax = fromDisplayTimeToDate(m_visibleTimeRangeMax());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QDateTime RimSummaryTimeAxisProperties::fromDisplayTimeToDate(double displayTime)
{
    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType(rimSummaryPlot);
    time_t startOfSimulation = rimSummaryPlot->firstTimeStepOfFirstCurve();

    time_t secsSinceSimulationStart = displayTime/fromTimeTToDisplayUnitScale();
    QDateTime date;
    date.setTime_t(startOfSimulation + secsSinceSimulationStart);

    return date;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::fromDateToDisplayTime(const QDateTime& displayTime)
{
    time_t secsSinceEpoc = displayTime.toTime_t();

    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType(rimSummaryPlot);
    time_t startOfSimulation = rimSummaryPlot->firstTimeStepOfFirstCurve();

    return fromTimeTToDisplayUnitScale()*(secsSinceEpoc - startOfSimulation);
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
double RimSummaryTimeAxisProperties::fromTimeTToDisplayUnitScale()
{
    double scale = 1.0;
    switch (m_timeUnit())
    {
        case SECONDS:
        break;
        case MINUTES:
        scale = 1.0/60.0;
        break;
        case HOURS:
        scale = 1.0/(60.0*60.0);
        break;
        case DAYS:
        scale = 1.0/(60.0*60.0*24.0);
        break;
        case YEARS:
        scale = 1.0/31556952.0;
        break;
        default:
        CVF_ASSERT(false);
        break;
    }

    return scale;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
double RimSummaryTimeAxisProperties::fromDaysToDisplayUnitScale()
{
    double scale = 1.0;
    switch (m_timeUnit())
    {
    case SECONDS:
        scale = 60.0 * 60.0 * 24.0;
        break;
    case MINUTES:
        scale = 60.0 * 24.0;
        break;
    case HOURS:
        scale = 24.0;
        break;
    case DAYS:
        break;
    case YEARS:
        scale = 1.0/365.2425;
        break;
    default:
        CVF_ASSERT(false);
        break;
    }

    return scale;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup& titleGroup = *(uiOrdering.addNewGroup("Axis Title"));
    titleGroup.add(&showTitle);
    titleGroup.add(&title);
    titleGroup.add(&titlePositionEnum);
    titleGroup.add(&fontSize);

    caf::PdmUiGroup* timeGroup =  uiOrdering.addNewGroup("Time Values");
    timeGroup->add(&m_timeMode);
    if (m_timeMode() == DATE)
    {
        timeGroup->add( &m_visibleDateRangeMax);
        timeGroup->add(&m_visibleDateRangeMin);
    }
    else
    {
        timeGroup->add(&m_timeUnit);     
        timeGroup->add(&m_visibleTimeRangeMax);
        timeGroup->add(&m_visibleTimeRangeMin);
    }

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryTimeAxisProperties::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    RimSummaryPlot* rimSummaryPlot = nullptr;
    this->firstAncestorOrThisOfType(rimSummaryPlot);
    if (!rimSummaryPlot) return;

    if (changedField == &m_visibleDateRangeMax)
    {
        QDateTime test = newValue.toDateTime();
        if (!test.isValid())
        {
            m_visibleDateRangeMax = oldValue.toDateTime();
        }

        updateTimeVisibleRange();
        rimSummaryPlot->disableAutoZoom();
    }
    else if (changedField == &m_visibleDateRangeMin)
    {
        QDateTime test = newValue.toDateTime();
        if (!test.isValid())
        {
            m_visibleDateRangeMin = oldValue.toDateTime();
        }

        updateTimeVisibleRange();
        rimSummaryPlot->disableAutoZoom();
    }
    else if (changedField == &m_visibleTimeRangeMin || changedField == &m_visibleTimeRangeMax)
    {
        updateDateVisibleRange();
        rimSummaryPlot->disableAutoZoom();
    }
    else if (changedField == &m_timeMode)
    {
        rimSummaryPlot->loadDataAndUpdate();
    }
    else if (changedField == &m_timeUnit)
    {
        updateTimeVisibleRange(); // Use the stored max min dates to update the visible time range to new unit
        rimSummaryPlot->loadDataAndUpdate();
        updateDateVisibleRange();
    }

    rimSummaryPlot->updateAxes();
}

