/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RimAsciiDataCurve.h"

#include "RiaApplication.h"

#include "RifReaderEclipseSummary.h"

#include "RiaDefines.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryFilter.h"
#include "RimSummaryPlot.h"
#include "RimSummaryTimeAxisProperties.h"

#include "RiuQwtPlotCurve.h"
#include "RiuSummaryQwtPlot.h"

#include "cafPdmUiComboBoxEditor.h"
#include "cafPdmUiListEditor.h"
#include "cafPdmUiTreeOrdering.h"

#include "qwt_date.h"


CAF_PDM_SOURCE_INIT(RimAsciiDataCurve, "AsciiDataCurve");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAsciiDataCurve::RimAsciiDataCurve()
{
    CAF_PDM_InitObject("ASCII Data Curve", ":/SummaryCurve16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_plotAxis,  "PlotAxis",  "Axis", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_timeSteps, "TimeSteps", "Time Steps", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_values,    "Values",    "Values", "", "", "");
    CAF_PDM_InitFieldNoDefault(&m_title,     "Title",     "Title", "", "", "");

    m_symbolSkipPixelDistance = 10.0f;
    m_curveThickness = 2;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimAsciiDataCurve::~RimAsciiDataCurve()
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<double> RimAsciiDataCurve::yValues() const
{
    return m_values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
const std::vector<time_t>& RimAsciiDataCurve::timeSteps() const
{
    static std::vector<time_t> timeSteps;
    timeSteps.clear();

    for (const QDateTime& dateTime : m_timeSteps())
    {
        timeSteps.push_back(dateTime.toTime_t());
    }

    return timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setYAxis(RiaDefines::PlotAxis plotAxis)
{
    m_plotAxis = plotAxis;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaDefines::PlotAxis RimAsciiDataCurve::yAxis() const
{
    return m_plotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimAsciiDataCurve::createCurveAutoName()
{
    return m_title();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::updateZoomInParentPlot()
{
    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfType(plot);

    plot->updateZoomInQwt(); 
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::onLoadDataAndUpdate(bool updateParentPlot)
{
    this->RimPlotCurve::updateCurvePresentation(updateParentPlot);

    if (isCurveVisible())
    {
        std::vector<time_t> dateTimes = this->timeSteps();
        std::vector<double> values = this->yValues();

        RimSummaryPlot* plot = nullptr;
        firstAncestorOrThisOfType(plot);
        bool isLogCurve = plot->isLogarithmicScaleEnabled(this->yAxis());

        if (dateTimes.size() > 0 && dateTimes.size() == values.size())
        {
            if (plot->timeAxisProperties()->timeMode() == RimSummaryTimeAxisProperties::DATE)
            {
                m_qwtPlotCurve->setSamplesFromTimeTAndYValues(dateTimes, values, isLogCurve);
            }
            else
            {
                double timeScale  = plot->timeAxisProperties()->fromTimeTToDisplayUnitScale();

                std::vector<double> times;
                if ( dateTimes.size() )
                {
                    time_t startDate = dateTimes[0];
                    for ( time_t& date: dateTimes )
                    {
                        times.push_back(timeScale*(date - startDate));
                    }
                }

                m_qwtPlotCurve->setSamplesFromXValuesAndYValues(times, values, isLogCurve);
            }
           
        }
        else
        {
            m_qwtPlotCurve->setSamplesFromTimeTAndYValues(std::vector<time_t>(), std::vector<double>(), isLogCurve);
        }

        updateZoomInParentPlot();

        if (m_parentQwtPlot) m_parentQwtPlot->replot();
    }

    updateQwtPlotAxis();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    RimPlotCurve::updateOptionSensitivity();

    uiOrdering.add(&m_plotAxis);

    caf::PdmUiGroup* appearanceGroup = uiOrdering.addNewGroup("Appearance");
    RimPlotCurve::appearanceUiOrdering(*appearanceGroup);

    caf::PdmUiGroup* nameGroup = uiOrdering.addNewGroup("Curve Name");
    nameGroup->setCollapsedByDefault(true);
    nameGroup->add(&m_showLegend);
    RimPlotCurve::curveNameUiOrdering(*nameGroup);

    uiOrdering.skipRemainingFields(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::updateQwtPlotAxis()
{
    if (m_qwtPlotCurve)
    {
        if (this->yAxis() == RiaDefines::PLOT_AXIS_LEFT)
        {
            m_qwtPlotCurve->setYAxis(QwtPlot::yLeft);
        }
        else
        {
            m_qwtPlotCurve->setYAxis(QwtPlot::yRight);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setTimeSteps(const std::vector<QDateTime>& timeSteps)
{
    m_timeSteps = timeSteps;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setValues(const std::vector<double>& values)
{
    m_values = values;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::setTitle(const QString& title)
{
    m_title = title;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimAsciiDataCurve::curveData(std::vector<QDateTime>* timeSteps, std::vector<double>* values) const
{
    CVF_ASSERT(timeSteps && values);

    *timeSteps = m_timeSteps();
    *values = m_values();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimAsciiDataCurve::fieldChangedByUi(const caf::PdmFieldHandle * changedField, const QVariant & oldValue, const QVariant & newValue)
{
    RimPlotCurve::fieldChangedByUi(changedField, oldValue, newValue);

    RimSummaryPlot* plot = nullptr;
    firstAncestorOrThisOfTypeAsserted(plot);

    if (changedField == &m_plotAxis)
    {
        updateQwtPlotAxis();
        plot->updateAxes();
    }
    if (changedField == &m_showCurve)
    {
        plot->updateAxes();
    }
}

