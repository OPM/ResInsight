/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "qwt_plot_marker.h"

#include <memory>
#include <vector>

#include <QPointer>

class QString;
class QwtPlot;

class RiuPlotAnnotationTool
{
public:
    RiuPlotAnnotationTool() {};
    ~RiuPlotAnnotationTool();

    void attachFormationNames(QwtPlot* plot, const std::vector<QString>& names, const std::vector<std::pair<double, double>> yPositions);
    void attachWellPicks(QwtPlot* plot, const std::vector<QString>& names, const std::vector<double> yPositions);
    void detachAllAnnotations();

private:
    static void horizontalDashedLine(QwtPlotMarker* line, const QString& name, double yValue);

private:
    QPointer<QwtPlot> m_plot;
    std::vector<QwtPlotMarker*> m_markers;
};
