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

#include "RiaDefines.h"
#include "RimSummaryCurve.h"
#include "RimSummaryAxisProperties.h"
#include "RimAsciiDataCurve.h"

#include "RiuSummaryQwtPlot.h"
#include "RiuSummaryVectorDescriptionMap.h"

#include "qwt_plot_curve.h"
#include "qwt_scale_draw.h"
#include "qwt_scale_engine.h"

#include <set>
#include <string>
#include <cmath>

//--------------------------------------------------------------------------------------------------
// e    format as [-]9.9e[+|-]999
// E    format as[-]9.9E[+| -]999
// f    format as[-]9.9
// g    use e or f format, whichever is the most concise
// G    use E or f format, whichever is the most concise

//--------------------------------------------------------------------------------------------------
class DecimalScaleDraw : public QwtScaleDraw
{
public:
    DecimalScaleDraw(double scaleFactor, int numberOfDecimals)
    {
        m_scaleFactor = scaleFactor;
        m_numberOfDecimals = numberOfDecimals;
    }

    QwtText label(double value) const override
    {
        if (qFuzzyCompare(scaledValue(value) + 1.0, 1.0))
            value = 0.0;

        return QString::number(scaledValue(value), 'f', m_numberOfDecimals);
    }

private:
    double scaledValue(double value) const
    {
        return value / m_scaleFactor;
    }

    double  m_scaleFactor;
    int     m_numberOfDecimals;
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
class ScientificScaleDraw : public QwtScaleDraw
{
public:
    ScientificScaleDraw(double scaleFactor, int numberOfDecimals)
    {
        m_scaleFactor = scaleFactor;
        m_numberOfDecimals = numberOfDecimals;
    }

    QwtText label(double value) const override
    {
        if (qFuzzyCompare(scaledValue(value) + 1.0, 1.0))
            value = 0.0;

        return QString::number(scaledValue(value), 'e', m_numberOfDecimals);
    }

private:
    double scaledValue(double value) const
    {
        return value / m_scaleFactor;
    }

    double  m_scaleFactor;
    int     m_numberOfDecimals;
};


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotYAxisFormatter::RimSummaryPlotYAxisFormatter(RimSummaryAxisProperties* axisProperties,
    const std::vector<RimSummaryCurve*>& summaryCurves,
    const std::vector<RimAsciiDataCurve*>& asciiCurves,
    const std::set<QString>& timeHistoryCurveQuantities)
:   m_axisProperties(axisProperties),
    m_summaryCurves(summaryCurves),
    m_asciiDataCurves(asciiCurves),
    m_timeHistoryCurveQuantities(timeHistoryCurveQuantities)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotYAxisFormatter::applyYAxisPropertiesToPlot(RiuSummaryQwtPlot* qwtPlot)
{
    if (!qwtPlot) return;

    {
        QString axisTitle = m_axisProperties->customTitle;
        if (m_axisProperties->useAutoTitle()) axisTitle = autoAxisTitle();

        QwtText axisTitleY = qwtPlot->axisTitle(m_axisProperties->qwtPlotAxisType());

        QFont axisTitleYFont = axisTitleY.font();
        axisTitleYFont.setBold(true);
        axisTitleYFont.setPixelSize(m_axisProperties->titleFontSize);
        axisTitleY.setFont(axisTitleYFont);

        axisTitleY.setText(axisTitle);

        switch (m_axisProperties->titlePositionEnum())
        {
            case RimSummaryAxisProperties::AXIS_TITLE_CENTER:
            axisTitleY.setRenderFlags(Qt::AlignCenter);
            break;
            case RimSummaryAxisProperties::AXIS_TITLE_END:
            axisTitleY.setRenderFlags(Qt::AlignRight);
            break;
        }

        qwtPlot->setAxisTitle(m_axisProperties->qwtPlotAxisType(), axisTitleY);
    }

    {
        QFont yAxisFont = qwtPlot->axisFont(m_axisProperties->qwtPlotAxisType());
        yAxisFont.setBold(false);
        yAxisFont.setPixelSize(m_axisProperties->valuesFontSize);
        qwtPlot->setAxisFont(m_axisProperties->qwtPlotAxisType(), yAxisFont);
    }

    {
        if (m_axisProperties->numberFormat == RimSummaryAxisProperties::NUMBER_FORMAT_AUTO)
        {
            qwtPlot->setAxisScaleDraw(m_axisProperties->qwtPlotAxisType(), new QwtScaleDraw);
        }
        else if (m_axisProperties->numberFormat == RimSummaryAxisProperties::NUMBER_FORMAT_DECIMAL)
        {
            qwtPlot->setAxisScaleDraw(m_axisProperties->qwtPlotAxisType(), 
                                      new DecimalScaleDraw(m_axisProperties->scaleFactor(), m_axisProperties->numberOfDecimals()));
        }
        else if (m_axisProperties->numberFormat == RimSummaryAxisProperties::NUMBER_FORMAT_SCIENTIFIC)
        {
            qwtPlot->setAxisScaleDraw(m_axisProperties->qwtPlotAxisType(), 
                                      new ScientificScaleDraw(m_axisProperties->scaleFactor(), m_axisProperties->numberOfDecimals()));
        }
    }

    {
        if (m_axisProperties->isLogarithmicScaleEnabled)
        {
            QwtLogScaleEngine* currentScaleEngine = dynamic_cast<QwtLogScaleEngine*>(qwtPlot->axisScaleEngine(m_axisProperties->qwtPlotAxisType()));
            if (!currentScaleEngine)
            {
                qwtPlot->setAxisScaleEngine(m_axisProperties->qwtPlotAxisType(), new QwtLogScaleEngine);
                qwtPlot->setAxisMaxMinor(m_axisProperties->qwtPlotAxisType(), 5);
            }

        }
        else
        {
            QwtLinearScaleEngine* currentScaleEngine = dynamic_cast<QwtLinearScaleEngine*>(qwtPlot->axisScaleEngine(m_axisProperties->qwtPlotAxisType()));
            if (!currentScaleEngine)
            {
                qwtPlot->setAxisScaleEngine(m_axisProperties->qwtPlotAxisType(), new QwtLinearScaleEngine);
                qwtPlot->setAxisMaxMinor(m_axisProperties->qwtPlotAxisType(), 3);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimSummaryPlotYAxisFormatter::autoAxisTitle() const
{
    std::map<std::string, std::set<std::string>> unitToQuantityNameMap;

    for (RimSummaryCurve* rimCurve : m_summaryCurves)
    {
        RifEclipseSummaryAddress sumAddress;
        std::string              unitText;

        if (rimCurve->summaryAddressY().category() == RifEclipseSummaryAddress::SUMMARY_ENSEMBLE_STATISTICS)
        {
            continue;
        }
        else if (m_axisProperties->plotAxisType() == RiaDefines::PLOT_AXIS_BOTTOM)
        {
            sumAddress = rimCurve->summaryAddressX();
            unitText   = rimCurve->unitNameX();
        }
        else if (rimCurve->axisY() == this->m_axisProperties->plotAxisType())
        {
            sumAddress = rimCurve->summaryAddressY();
            unitText   = rimCurve->unitNameY();
        }
        else
        {
            continue;
        }

        std::string quantityNameForDisplay;
        {
            std::string quantityName = sumAddress.quantityName();

            if (sumAddress.category() == RifEclipseSummaryAddress::SUMMARY_CALCULATED)
            {
                quantityNameForDisplay = shortCalculationName(quantityName);
            }
            else
            {
                if (m_axisProperties->showDescription())
                {
                    quantityNameForDisplay = RiuSummaryVectorDescriptionMap::instance()->vectorLongName(quantityName, true);
                }

                if (m_axisProperties->showAcronym())
                {
                    if (!quantityNameForDisplay.empty())
                    {
                        quantityNameForDisplay += " (";
                        quantityNameForDisplay += quantityName;
                        quantityNameForDisplay += ")";
                    }
                    else
                    {
                        quantityNameForDisplay += quantityName;
                    }

                }
            }

            unitToQuantityNameMap[unitText].insert(quantityNameForDisplay);
        }
    }

    QString assembledYAxisText;
    QString scaleFactorText = "";

    if (m_axisProperties->numberFormat() != RimSummaryAxisProperties::NUMBER_FORMAT_AUTO)
    {
        if (m_axisProperties->scaleFactor() != 1.0)
        {
            int exponent    = std::log10(m_axisProperties->scaleFactor());
            scaleFactorText = QString(" x 10<sup>%1</sup> ").arg(QString::number(exponent));
        }
    }

    for (auto unitIt : unitToQuantityNameMap)
    {
        for (const auto& quantIt : unitIt.second)
        {
            assembledYAxisText += QString::fromStdString(quantIt) + " ";
        }

        if (m_axisProperties->showUnitText() && !unitIt.first.empty())
        {
            assembledYAxisText += "[" + QString::fromStdString(unitIt.first) + scaleFactorText + "] ";
        }
    }

    if (m_timeHistoryCurveQuantities.size() > 0)
    {
        if (!assembledYAxisText.isEmpty())
        {
            assembledYAxisText += " : ";
        }

        for (auto timeQuantity : m_timeHistoryCurveQuantities)
        {
            assembledYAxisText += timeQuantity + " ";
        }
    }

    return assembledYAxisText;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::string RimSummaryPlotYAxisFormatter::shortCalculationName(const std::string& calculationName)
{
    QString calculationShortName = QString::fromStdString(calculationName);

    int indexOfFirstSpace = calculationShortName.indexOf(' ');
    if (indexOfFirstSpace > -1 && indexOfFirstSpace < calculationShortName.size())
    {
        calculationShortName = calculationShortName.left(indexOfFirstSpace);
    }

    return calculationShortName.toStdString();
}




//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSummaryPlotYAxisRangeCalculator::RimSummaryPlotYAxisRangeCalculator(
    const std::vector<QwtPlotCurve*>& qwtCurves,
    const std::vector<double>& yValuesForAllCurves)
    : 
    m_singleCurves(qwtCurves),
    m_yValuesForAllCurves(yValuesForAllCurves)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSummaryPlotYAxisRangeCalculator::computeYRange(double* min, double* max) const
{
    double minValue = HUGE_VAL;
    double maxValue = -HUGE_VAL;

    for (QwtPlotCurve* curve : m_singleCurves)
    {
        double minCurveValue = HUGE_VAL;
        double maxCurveValue = -HUGE_VAL;

        if (curveValueRangeY(curve, &minCurveValue, &maxCurveValue))
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
        minValue = RiaDefines::minimumDefaultValuePlot();
        maxValue = RiaDefines::maximumDefaultValuePlot();
    }

    // For logarithmic auto scaling, compute positive curve value closest to zero and use
    // this value as the plot visible minimum

    double pos = HUGE_VAL;
    double neg = -HUGE_VAL;

    RigStatisticsCalculator::posNegClosestToZero(m_yValuesForAllCurves, pos, neg);

    if (pos != HUGE_VAL)
    {
        minValue = pos;
    }

    *min = minValue;
    *max = maxValue;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimSummaryPlotYAxisRangeCalculator::curveValueRangeY(const QwtPlotCurve* qwtCurve, double* min, double* max) const
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

