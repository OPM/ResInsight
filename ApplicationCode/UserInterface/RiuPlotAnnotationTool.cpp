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

#include "RiuPlotAnnotationTool.h"

#include <QString>

#include "qwt_plot.h"

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachFormationNames(const std::vector<QString>& names, const std::vector<double> yPositions)
{
    if (names.size() != yPositions.size()) return;

    QPen curvePen;
    curvePen.setStyle(Qt::DashLine);
    curvePen.setColor(Qt::gray);
    curvePen.setWidth(1);

    for (size_t i = 0; i < names.size(); i++)
    {
        std::unique_ptr<QwtPlotMarker> line(std::make_unique<QwtPlotMarker>());
   
        line->setLineStyle(QwtPlotMarker::HLine);
        line->setLinePen(curvePen);
        line->setYValue(yPositions[i]);
        line->setLabel(names[i]);
        line->setLabelAlignment(Qt::AlignLeft | Qt::AlignBottom);

        line->attach(m_plot);

        m_markers.push_back(std::move(line));
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::detachAllAnnotations()
{
    for (size_t i = 0; i < m_markers.size(); i++)
    {
        m_markers[i]->detach();
    }
    m_markers.clear();
}
