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

#pragma once

#include "qwt_symbol.h"
#include "qwt_plot_curve.h"
#include "qwt_plot_intervalcurve.h"

class RiuErrorBarsQwtPlotCurve;

//==================================================================================================
//
// If infinite data is present in the curve data, Qwt is not able to draw a nice curve.
// This class assumes that inf data is removed, and segments to be draw are indicated by start/stop indices into curve data.
//
// Single values in the curve are drawn using a CrossX symbol
//
//  Here you can see the curve segments visualized. Curve segments are drawn between vector indices.
//
//  0 - 1
//  5 - 7
//  9 -10
//
//                 /                    ^
//                /                   /   \
//  Curve        /                   /     \       -----        X
//
//  Values     1.0|2.0|inf|inf|inf|1.0|2.0|1.0|inf|1.0|1.0|inf|1.0|inf
//  Vec index   0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10| 11| 12| 13
//==================================================================================================
class RiuLineSegmentQwtPlotCurve : public QwtPlotCurve
{
public:
    explicit RiuLineSegmentQwtPlotCurve(const QString &title = QString::null);
    virtual ~RiuLineSegmentQwtPlotCurve();

    void         setSamplesFromXValuesAndYValues(const std::vector<double>& xValues,
                                                 const std::vector<double>& yValues,
                                                 bool keepOnlyPositiveValues);

    void         setSamplesFromXValuesAndYValues(const std::vector<double>& xValues,
                                                 const std::vector<double>& yValues,
                                                 const std::vector<double>& yErrorValues,
                                                 bool keepOnlyPositiveValues);
    
    void         setSamplesFromDatesAndYValues(const std::vector<QDateTime>& dateTimes,
                                               const std::vector<double>& yValues,
                                               bool keepOnlyPositiveValues);

    void         setSamplesFromTimeTAndYValues(const std::vector<time_t>& dateTimes,
                                               const std::vector<double>& yValues,
                                               bool keepOnlyPositiveValues);

    void         setSamplesFromTimeTAndYValues(const std::vector<time_t>& dateTimes,
                                               const std::vector<double>& yValues,
                                               const std::vector<double>& yErrorValues,
                                               bool keepOnlyPositiveValues);

    void         setLineSegmentStartStopIndices(const std::vector< std::pair<size_t, size_t> >& lineSegmentStartStopIndices);

    void         setSymbolSkipPixelDistance(float distance);

    void         attach(QwtPlot *plot);
    void         detach();
    void         showErrorBars(bool show);
    void         setErrorBarsColor(QColor color);

protected:
    virtual void drawCurve(QPainter* p, int style,
                            const QwtScaleMap& xMap, const QwtScaleMap& yMap,
                            const QRectF& canvasRect, 
                            int from, int to) const;
 

    virtual void drawSymbols(QPainter *p, const QwtSymbol &symbol, 
                             const QwtScaleMap &xMap, 
                             const QwtScaleMap &yMap, 
                             const QRectF &canvasRect, 
                             int from, int to) const override;

private:
    static std::vector<double>  fromQDateTime(const std::vector<QDateTime>& dateTimes);
    static std::vector<double>  fromTime_t(const std::vector<time_t>& timeSteps);

private:
    std::vector< std::pair<size_t, size_t> > m_polyLineStartStopIndices;
    float                                    m_symbolSkipPixelDistance;

    bool                    m_showErrorBars;
    QwtPlotIntervalCurve*   m_errorBars;
    QwtPlot*                m_attachedToPlot;
};

//--------------------------------------------------------------------------------------------------
/// This class overrides renderSymbols to draw symbols and labels.
/// The label is only visible in the legend, while it is clipped in the plot.
/// Therefore the method RiuLineSegmentQwtPlotCurve::drawSymbols also draw labels to have labels
/// in the plot as well.
//--------------------------------------------------------------------------------------------------
class RiuCurveQwtSymbol : public QwtSymbol
{
public:
    RiuCurveQwtSymbol(QwtSymbol::Style style, const QString& label);

    virtual void renderSymbols(QPainter *painter, const QPointF *points, int numPoints) const override;

    QString label() const { return m_label; }

private:
    QString m_label;
};
