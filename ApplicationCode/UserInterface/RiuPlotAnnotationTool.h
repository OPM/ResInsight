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
    enum FormationDisplay
    {
        NONE                    = 0x00,
        DARK_LINES              = 0x01,
        COLORED_LINES           = 0x02,
        COLOR_SHADING           = 0x04,
        COLOR_SHADING_AND_LINES = 0x05
    };

public:
    RiuPlotAnnotationTool(){};
    ~RiuPlotAnnotationTool();

    void attachFormationNames( QwtPlot*                                     plot,
                               const std::vector<QString>&                  names,
                               const std::pair<double, double>              xRange,
                               const std::vector<std::pair<double, double>> yPositions,
                               FormationDisplay                             formationDisplay,
                               bool                                         showNames = true );
    void attachWellPicks( QwtPlot* plot, const std::vector<QString>& names, const std::vector<double> yPositions );

    void attachAnnotationLine( QwtPlot* plot, const QColor& color, const QString& annotationText, const double yPosition );

    void detachAllAnnotations();

private:
    static void horizontalDashedLine( QwtPlotMarker* line, const QString& name, double yValue );
    static void horizontalDashedLineWithColor(
        QwtPlotMarker* line, const QColor& color, const QColor& textColor, const QString& name, double yValue );

private:
    QPointer<QwtPlot>         m_plot;
    std::vector<QwtPlotItem*> m_markers;
};
