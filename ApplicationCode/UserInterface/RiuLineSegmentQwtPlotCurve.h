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

#include "qwt_plot_curve.h"

#include <vector>

class RigWellLogCurveData;

//==================================================================================================
//
// The PlotCurve class is able to draw a curve using line segments. If inf data is present 
// in the curve data, Qwt is not able to draw a nice curve. This class assumes that inf data is removed, 
// and segments to be draw are indicated by start/stop indices into curve data.
//
//  Here you can see the curve segments visualized. Curve segments are drawn between vector indices.
//
//  0 - 1
//  5 - 7
//  9 -10
//
//                  *                   *
//                /                   /   \
//  Curve       *                   *       *      *---*
//
//  Values     1.0|2.0|inf|inf|inf|1.0|2.0|1.0|inf|1.0|1.0
//  Vec index   0 | 1 | 2 | 3 | 4 | 5 | 6 | 7 | 8 | 9 | 10
//==================================================================================================
class RiuLineSegmentQwtPlotCurve : public QwtPlotCurve
{
public:
    RiuLineSegmentQwtPlotCurve();
    virtual ~RiuLineSegmentQwtPlotCurve();

    void setCurveData(const RigWellLogCurveData* curveData);
    void setCurveData(const std::vector<double>& xValues, const std::vector<double>& yValues, const std::vector< std::pair<size_t, size_t> >& lineSegmentStartStopIndices);

protected:
    virtual void drawCurve(QPainter* p, int style,
                            const QwtScaleMap& xMap, const QwtScaleMap& yMap,
                            const QRectF& canvasRect, int from, int to) const;
 
private:
    std::vector< std::pair<size_t, size_t> >    m_polyLineStartStopIndices;
};
