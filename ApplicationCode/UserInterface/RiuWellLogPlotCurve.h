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
///  
///  
//==================================================================================================
class RiuWellLogPlotCurve : public QwtPlotCurve
{
public:

    RiuWellLogPlotCurve();
    virtual ~RiuWellLogPlotCurve();

    void setCurveData(const RigWellLogCurveData* curveData);

protected:

    virtual void drawCurve(QPainter* p, int style,
                            const QwtScaleMap& xMap, const QwtScaleMap& yMap,
                            const QRectF& canvasRect, int from, int to) const;
 
private:
    std::vector< std::pair<size_t, size_t> >    m_polyLineStartStopIndices;
};
