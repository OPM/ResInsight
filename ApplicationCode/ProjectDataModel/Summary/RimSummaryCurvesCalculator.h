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
#pragma once

#include <vector>

#include <QString>

class RimSummaryCurve;
class RimSummaryCurveFilter;
class RimSummaryYAxisProperties;

class RiuSummaryQwtPlot;

class QwtPlotCurve;

class RimSummaryCurvesCalculator
{
public:
    RimSummaryCurvesCalculator(RimSummaryYAxisProperties* axisProperties,
        const std::vector<RimSummaryCurve*>& curves,
        const std::vector<RimSummaryCurveFilter*>& curveFilters);

    RimSummaryCurvesCalculator(RimSummaryYAxisProperties* axisProperties,
        const std::vector<RimSummaryCurve*>& curves);

    void    applyPropertiesToPlot(RiuSummaryQwtPlot* qwtPlot);
    void    computeYRange(double* min, double* max) const;

private:
    QString autoAxisTitle() const;
    bool    curveValueRangeY(const QwtPlotCurve* qwtCurve, double* min, double* max) const;

private:
    RimSummaryYAxisProperties*          m_axisProperties;
    std::vector<RimSummaryCurve*>       m_singleCurves;
    std::vector<RimSummaryCurveFilter*> m_curveFilters;
};

