/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-2018 Statoil ASA
//  Copyright (C) 2019-     Equinor ASA
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

#include "qwt_plot.h"

class QwtPlotCurve;

//==================================================================================================
///
///
//==================================================================================================
class RimPlotAxisLogRangeCalculator
{
public:
    RimPlotAxisLogRangeCalculator( QwtPlot::Axis axis, const std::vector<const QwtPlotCurve*>& qwtCurves );

    void computeAxisRange( double* minPositive, double* max ) const;

private:
    bool curveValueRange( const QwtPlotCurve* qwtCurve, double* minPositive, double* max ) const;

private:
    QwtPlot::Axis                          m_axis;
    const std::vector<const QwtPlotCurve*> m_curves;
};
