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

#include "cvfMath.h"
#include "qwt_plot.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuPlotAnnotationTool::~RiuPlotAnnotationTool()
{
    detachAllAnnotations();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachFormationNames(QwtPlot*                                     plot,
                                                 const std::vector<QString>&                  names,
                                                 const std::vector<std::pair<double, double>> yPositions,
                                                 bool                                         showNames)
{
    detachAllAnnotations();

    if (names.size() != yPositions.size()) return;
    m_plot = plot;

    double delta = 0.5;

    for (size_t i = 0; i < names.size(); i++)
    {
        QwtPlotMarker* line(new QwtPlotMarker());

        QString name;
        if (showNames)
        {
            name = names[i];
            if (names[i].toLower().indexOf("top") == -1)
            {
                name += " Top";
            }
        }

        RiuPlotAnnotationTool::horizontalDashedLine(line, name, yPositions[i].first);

        line->attach(m_plot);
        m_markers.push_back(std::move(line));

        if ((i != names.size() - 1) && cvf::Math::abs(yPositions[i].second - yPositions[i + 1].first) > delta)
        {
            QwtPlotMarker* bottomLine(new QwtPlotMarker());
            RiuPlotAnnotationTool::horizontalDashedLine(bottomLine, QString(), yPositions[i].second);

            bottomLine->attach(m_plot);
            m_markers.push_back(std::move(bottomLine));
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachWellPicks(QwtPlot*                    plot,
                                            const std::vector<QString>& names,
                                            const std::vector<double>   yPositions)
{
    detachAllAnnotations();

    if (names.size() != yPositions.size()) return;
    m_plot = plot;

    for (size_t i = 0; i < names.size(); i++)
    {
        QwtPlotMarker* line(new QwtPlotMarker());
        RiuPlotAnnotationTool::horizontalDashedLine(line, names[i], yPositions[i]);
        line->attach(m_plot);
        m_markers.push_back(std::move(line));
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::attachAnnotationLine(QwtPlot*       plot,
                                                 const QColor&  color,
                                                 const QString& annotationText,
                                                 const double   yPosition)
{
    m_plot = plot;

    QwtPlotMarker* line(new QwtPlotMarker());
    RiuPlotAnnotationTool::horizontalDashedLineWithColor(line, color, annotationText, yPosition);
    line->attach(m_plot);
    m_markers.push_back(std::move(line));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::detachAllAnnotations()
{
    if (m_plot)
    {
        for (size_t i = 0; i < m_markers.size(); i++)
        {
            m_markers[i]->detach();
            delete m_markers[i];
        }
    }
    m_markers.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::horizontalDashedLine(QwtPlotMarker* line, const QString& name, double yValue)
{
    horizontalDashedLineWithColor(line, QColor(0, 0, 100), name, yValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuPlotAnnotationTool::horizontalDashedLineWithColor(QwtPlotMarker* line,
                                                          const QColor&  color,
                                                          const QString& name,
                                                          double         yValue)
{
    QPen curvePen;
    curvePen.setStyle(Qt::DashLine);
    curvePen.setColor(color);
    curvePen.setWidth(1);

    line->setLineStyle(QwtPlotMarker::HLine);
    line->setLinePen(curvePen);
    line->setYValue(yValue);
    line->setLabel(name);
    line->setLabelAlignment(Qt::AlignRight | Qt::AlignBottom);
}
