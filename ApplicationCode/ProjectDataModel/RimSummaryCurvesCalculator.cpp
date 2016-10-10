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

#include "RimSummaryCurvesCalculator.h"

#include "RigStatisticsCalculator.h"

#include "RimDefines.h"
#include "RimSummaryCurve.h"
#include "RimSummaryCurveFilter.h"
#include "RimSummaryYAxisProperties.h"

#include "RiuSummaryQwtPlot.h"

#include "qwt_plot_curve.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"

#include <set>
#include <string>
#include <cmath>

//--------------------------------------------------------------------------------------------------
// e	format as [-]9.9e[+|-]999
// E	format as[-]9.9E[+| -]999
// f	format as[-]9.9
// g	use e or f format, whichever is the most concise
// G	use E or f format, whichever is the most concise

//--------------------------------------------------------------------------------------------------
class DecimalScaleDraw : public QwtScaleDraw
{
public:
    virtual QwtText label(double value) const override
    {
        if (qFuzzyCompare(value + 1.0, 1.0))
            value = 0.0;

        int precision = DecimalScaleDraw::calculatePrecision(value);

        return QString::number(value, 'f', precision);
    }

private:
    static int calculatePrecision(double value)
    {
        double absVal = fabs(value);
        if (1e-16 < absVal && absVal < 1.0e3)
        {
            int logVal = static_cast<int>(log10(absVal));
            int numDigitsAfterPoint = abs(logVal - 6);
            return numDigitsAfterPoint;
        }
        else
        {
            return 3;
        }
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class ScientificScaleDraw : public QwtScaleDraw
{

public:
    virtual QwtText label(double value) const override
    {
        if (qFuzzyCompare(value + 1.0, 1.0))
            value = 0.0;

        return QString::number(value, 'e', 2);
    }
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryCurvesCalculator::RimSummaryCurvesCalculator(RimSummaryYAxisProperties* axisProperties,
    const std::vector<RimSummaryCurve*>& curves, 
    const std::vector<RimSummaryCurveFilter*>& curveFilters)
:   m_axisProperties(axisProperties),
    m_singleCurves(curves),
    m_curveFilters(curveFilters)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesCalculator::applyPropertiesToPlot(RiuSummaryQwtPlot* m_qwtPlot)
{
    if (!m_qwtPlot) return;

    {
        QString axisTitle = m_axisProperties->customTitle;
        if (m_axisProperties->isAutoTitle) axisTitle = autoAxisTitle();

        QwtText axisTitleY = m_qwtPlot->axisTitle(m_axisProperties->axis());

        QFont axisTitleYFont = axisTitleY.font();
        axisTitleYFont.setBold(true);
        axisTitleYFont.setPixelSize(m_axisProperties->fontSize);
        axisTitleY.setFont(axisTitleYFont);

        axisTitleY.setText(axisTitle);
        m_qwtPlot->setAxisTitle(m_axisProperties->axis(), axisTitleY);
    }

    {
        QFont yAxisFont = m_qwtPlot->axisFont(m_axisProperties->axis());
        yAxisFont.setBold(false);
        yAxisFont.setPixelSize(m_axisProperties->fontSize);
        m_qwtPlot->setAxisFont(m_axisProperties->axis(), yAxisFont);
    }

    {
        if (m_axisProperties->numberFormat == RimSummaryYAxisProperties::NUMBER_FORMAT_AUTO)
        {
            m_qwtPlot->setAxisScaleDraw(m_axisProperties->axis(), new QwtScaleDraw);
        }
        else if (m_axisProperties->numberFormat == RimSummaryYAxisProperties::NUMBER_FORMAT_DECIMAL)
        {
            m_qwtPlot->setAxisScaleDraw(m_axisProperties->axis(), new DecimalScaleDraw);
        }
        else if (m_axisProperties->numberFormat == RimSummaryYAxisProperties::NUMBER_FORMAT_SCIENTIFIC)
        {
            m_qwtPlot->setAxisScaleDraw(m_axisProperties->axis(), new ScientificScaleDraw());
        }

    }

/*
    {
        if (m_axisProperties->isLogarithmicScaleEnabled)
        {
            QwtLogScaleEngine* currentScaleEngine = dynamic_cast<QwtLogScaleEngine*>(m_qwtPlot->axisScaleEngine(m_axisProperties->axis()));
            if (!currentScaleEngine)
            {
                m_qwtPlot->setAxisScaleEngine(m_axisProperties->axis(), new QwtLogScaleEngine);
            }

            m_qwtPlot->setAxisMaxMinor(m_axisProperties->axis(), 5);
        }
        else
        {
            QwtLinearScaleEngine* currentScaleEngine = dynamic_cast<QwtLinearScaleEngine*>(m_qwtPlot->axisScaleEngine(m_axisProperties->axis()));
            if (!currentScaleEngine)
            {
                m_qwtPlot->setAxisScaleEngine(m_axisProperties->axis(), new QwtLinearScaleEngine);
            }

            m_qwtPlot->setAxisMaxMinor(m_axisProperties->axis(), 3);
        }
    }
*/

    m_qwtPlot->setAxisScale(m_axisProperties->axis(), m_axisProperties->visibleRangeMin, m_axisProperties->visibleRangeMax);

/*
    {
        if (m_axisProperties->isAutoScaleEnabled)
        {
            double min = HUGE_VAL;
            double max = -HUGE_VAL;

            computeYRange(&min, &max);

            m_qwtPlot->setAxisScale(m_axisProperties->axis(), min, max);
        }
        else
        {
            m_qwtPlot->setAxisScale(m_axisProperties->axis(), m_axisProperties->visibleRangeMin, m_axisProperties->visibleRangeMax);
        }
    }
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryCurvesCalculator::autoAxisTitle() const
{
    std::set<std::string> unitNames;

    for (RimSummaryCurve* rimCurve : m_singleCurves)
    {
        if (rimCurve->isCurveVisible()) unitNames.insert(rimCurve->unitName());
    }

    for (RimSummaryCurveFilter* curveFilter : m_curveFilters)
    {
        std::set<std::string> filterUnitNames = curveFilter->unitNames();
        unitNames.insert(filterUnitNames.begin(), filterUnitNames.end());
    }

    QString assembledYAxisText;

    for (const std::string& unitName : unitNames)
    {
        assembledYAxisText += "[" + QString::fromStdString(unitName) + "] ";
    }

    return assembledYAxisText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryCurvesCalculator::computeYRange(double* min, double* max) const
{
    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    for (RimSummaryCurve* curve : m_singleCurves)
    {
        double minCurveValue = HUGE_VAL;
        double maxCurveValue = -HUGE_VAL;

        if (curve->isCurveVisible() && curveValueRangeY(curve->qwtPlotCurve(), &minCurveValue, &maxCurveValue))
        {
            if (minCurveValue < minValue)
            {
                minValue = minCurveValue;
            }

            if (maxCurveValue > maxValue)
            {
                maxValue = maxCurveValue;
            }
        }
    }

    if (minValue == HUGE_VAL)
    {
        minValue = RimDefines::minimumDefaultValuePlot();
        maxValue = RimDefines::maximumDefaultValuePlot();
    }

    if (m_axisProperties->isLogarithmicScaleEnabled)
    {
        // For logarithmic auto scaling, compute positive curve value closest to zero and use
        // this value as the plot visible minimum

        double pos = HUGE_VAL;
        double neg = -HUGE_VAL;

        for (RimSummaryCurve* curve : m_singleCurves)
        {
            if (curve->isCurveVisible())
            {
                RigStatisticsCalculator::posNegClosestToZero(curve->yPlotValues(), pos, neg);
            }
        }

        if (pos != HUGE_VAL)
        {
            minValue = pos;
        }
    }

    *min = minValue;
    *max = maxValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryCurvesCalculator::curveValueRangeY(const QwtPlotCurve* qwtCurve, double* min, double* max) const
{
    if (!qwtCurve) return false;

    if (qwtCurve->data()->size() < 1)
    {
        return false;
    }

    *min = qwtCurve->minYValue();
    *max = qwtCurve->maxYValue();

    return true;
}

