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

#include "RiaPlotDefines.h"

#include "cafColorTable.h"

#include "qwt_plot_marker.h"

#include <memory>
#include <vector>

#include <QColor>
#include <QPointer>

class QString;
class QwtPlot;
class RimPlotAxisAnnotation;

class RiuPlotAnnotationTool
{
public:
    RiuPlotAnnotationTool() {};
    ~RiuPlotAnnotationTool();

    void attachNamedRegions( QwtPlot*                                      plot,
                             const std::vector<QString>&                   names,
                             RiaDefines::Orientation                       orientation,
                             const std::vector<std::pair<double, double>>& regionRanges,
                             RiaDefines::RegionDisplay                     regionDisplay,
                             const caf::ColorTable&                        colorTable,
                             int                                           shadingAlphaByte,
                             bool                                          showNames   = true,
                             RiaDefines::TrackSpan                         trackSpan   = RiaDefines::TrackSpan::FULL_WIDTH,
                             const std::vector<Qt::BrushStyle>&            brushStyles = {},
                             int                                           fontSize    = 0 );
    void attachWellPicks( QwtPlot* plot, const std::vector<QString>& names, const std::vector<double>& yPositions );

    void attachAnnotationLine( QwtPlot*                plot,
                               const QColor&           color,
                               const QString&          annotationText,
                               Qt::PenStyle            penStyle,
                               const double            position,
                               RiaDefines::Orientation orientation,
                               Qt::Alignment           horizontalAlignment );

    void attachAnnotationRange( QwtPlot*                plot,
                                const QColor&           color,
                                const QString&          annotationText,
                                const double            rangeStart,
                                const double            rangeEnd,
                                RiaDefines::Orientation orientation );

    void horizontalRange( const QString&                  name,
                          const std::pair<double, double> yRange,
                          const QColor&                   color               = QColor( 0, 0, 100 ),
                          const QColor&                   textColor           = QColor( 0, 0, 100 ),
                          Qt::Alignment                   horizontalAlignment = Qt::AlignRight );

    void verticalRange( const QString&                  name,
                        const std::pair<double, double> xRange,
                        const QColor&                   color               = QColor( 0, 0, 100 ),
                        const QColor&                   textColor           = QColor( 0, 0, 100 ),
                        Qt::Alignment                   horizontalAlignment = Qt::AlignRight );

    void detachAllAnnotations();

    static Qt::Alignment textAlignment( RiaDefines::TextAlignment alignment );

private:
    static Qt::Alignment trackTextAlignment( RiaDefines::TrackSpan trackSpan );

    static void setLineProperties( QwtPlotMarker*          line,
                                   const QString&          name,
                                   RiaDefines::Orientation orientation,
                                   double                  linePosition,
                                   Qt::PenStyle            lineStyle           = Qt::DashLine,
                                   const QColor&           color               = QColor( 0, 0, 100 ),
                                   const QColor&           textColor           = QColor( 0, 0, 100 ),
                                   Qt::Alignment           horizontalAlignment = Qt::AlignRight,
                                   int                     fontSize            = 0 );

private:
    QPointer<QwtPlot>         m_plot;
    std::vector<QwtPlotItem*> m_plotItems;
};
