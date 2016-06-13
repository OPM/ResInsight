/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RiuLineSegmentQwtPlotCurve.h"

#include "qwt_symbol.h"
#include "RigCurveDataTools.h"
#include "qwt_date.h"


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuLineSegmentQwtPlotCurve::RiuLineSegmentQwtPlotCurve(const QString &title)
    : QwtPlotCurve(title)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiuLineSegmentQwtPlotCurve::~RiuLineSegmentQwtPlotCurve()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuLineSegmentQwtPlotCurve::setSamplesFromDateAndValues(const std::vector<QDateTime>& dateTimes, const std::vector<double>& timeHistoryValues)
{
    CVF_ASSERT(dateTimes.size() == timeHistoryValues.size());

    QPolygonF points;
    std::vector< std::pair<size_t, size_t> > filteredIntervals;
    {
        std::vector<double> filteredTimeHistoryValues;
        std::vector<QDateTime> filteredDateTimes;

        {
            std::vector< std::pair<size_t, size_t> > intervalsOfValidValues;
            RigCurveDataTools::calculateIntervalsOfValidValues(timeHistoryValues, &intervalsOfValidValues);

            RigCurveDataTools::getValuesByIntervals(timeHistoryValues, intervalsOfValidValues, &filteredTimeHistoryValues);
            RigCurveDataTools::getValuesByIntervals(dateTimes, intervalsOfValidValues, &filteredDateTimes);

            RigCurveDataTools::computePolyLineStartStopIndices(intervalsOfValidValues, &filteredIntervals);
        }


        for (size_t i = 0; i < filteredDateTimes.size(); i++)
        {
            double milliSecSinceEpoch = QwtDate::toDouble(filteredDateTimes[i]);
            points << QPointF(milliSecSinceEpoch, filteredTimeHistoryValues[i]);
        }
    }

    RiuLineSegmentQwtPlotCurve* plotCurve = new RiuLineSegmentQwtPlotCurve("Curve 1");

    this->setSamples(points);
    this->setLineSegmentStartStopIndices(filteredIntervals);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuLineSegmentQwtPlotCurve::drawCurve(QPainter* p, int style,
    const QwtScaleMap& xMap, const QwtScaleMap& yMap,
    const QRectF& canvasRect, int from, int to) const
{
    size_t intervalCount = m_polyLineStartStopIndices.size();
    if (intervalCount > 0)
    {
        for (size_t intIdx = 0; intIdx < intervalCount; intIdx++)
        {
            if (m_polyLineStartStopIndices[intIdx].first == m_polyLineStartStopIndices[intIdx].second)
            {
                // Use a symbol to draw a single value, as a single value will not be visible
                // when using QwtPlotCurve::drawCurve without symbols activated

                QwtSymbol symbol(QwtSymbol::XCross);
                symbol.setSize(10, 10);

                QwtPlotCurve::drawSymbols(p, symbol, xMap, yMap, canvasRect, (int) m_polyLineStartStopIndices[intIdx].first, (int) m_polyLineStartStopIndices[intIdx].second);
            }
            else
            {
                QwtPlotCurve::drawCurve(p, style, xMap, yMap, canvasRect, (int) m_polyLineStartStopIndices[intIdx].first, (int) m_polyLineStartStopIndices[intIdx].second);
            }
        }
    }
    else
    {
        QwtPlotCurve::drawCurve(p, style, xMap, yMap, canvasRect, from, to);
    }
};

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuLineSegmentQwtPlotCurve::setLineSegmentStartStopIndices(const std::vector< std::pair<size_t, size_t> >& lineSegmentStartStopIndices)
{
    m_polyLineStartStopIndices = lineSegmentStartStopIndices;
}
