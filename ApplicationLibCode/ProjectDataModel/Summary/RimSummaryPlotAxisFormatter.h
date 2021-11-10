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

#include <QString>

#include <set>
#include <vector>

class RimAsciiDataCurve;
class RimSummaryCurve;
class RimPlotAxisProperties;
class RiaSummaryCurveDefinition;

class RiuQwtPlotWidget;

class QwtPlotCurve;

class RimSummaryPlotAxisFormatter
{
public:
    RimSummaryPlotAxisFormatter( RimPlotAxisProperties*                        axisProperties,
                                 const std::vector<RimSummaryCurve*>&          summaryCurves,
                                 const std::vector<RiaSummaryCurveDefinition>& curveDefinitions,
                                 const std::vector<RimAsciiDataCurve*>&        asciiCurves,
                                 const std::set<QString>&                      timeHistoryCurveQuantities );

    void applyAxisPropertiesToPlot( RiuQwtPlotWidget* qwtPlot );

private:
    QString autoAxisTitle() const;

    static std::string shortCalculationName( const std::string& calculationName );

private:
    RimPlotAxisProperties*                       m_axisProperties;
    const std::vector<RimSummaryCurve*>          m_summaryCurves;
    const std::vector<RiaSummaryCurveDefinition> m_curveDefinitions;
    const std::vector<RimAsciiDataCurve*>        m_asciiDataCurves;
    const std::set<QString>                      m_timeHistoryCurveQuantities;
};
