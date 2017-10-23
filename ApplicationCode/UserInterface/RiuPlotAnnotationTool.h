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

class QString;
class QwtPlot;

class RiuPlotAnnotationTool
{
public:
    RiuPlotAnnotationTool(QwtPlot* plot) : m_plot(plot) {};

    void attachFormationNames(const std::vector<QString>& names, const std::vector<double> yPositions);
    void detachAllAnnotations();

private:
    QwtPlot* m_plot;
    std::vector<std::unique_ptr<QwtPlotMarker>> m_markers;
};
