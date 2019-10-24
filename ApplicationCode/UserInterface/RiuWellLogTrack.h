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

#include "qwt_plot.h"

#include "cafPdmPointer.h"

class RimWellLogTrack;

class QwtLegend;
class QwtPicker;
class QwtPlotGrid;
class QwtPlotMarker;

class QEvent;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuWellLogTrack : public QwtPlot
{
    Q_OBJECT

public:
    RiuWellLogTrack( RimWellLogTrack* plotTrackDefinition, QWidget* parent = nullptr );
    ~RiuWellLogTrack() override;

    void setDepthZoom( double minDepth, double maxDepth );
    void setDepthTitle( const QString& title );
    void setXTitle( const QString& title );

    void setXRange( double min, double max, QwtPlot::Axis axis = QwtPlot::xTop );

    bool   isRimTrackVisible();
    void   enableDepthAxisLabelsAndTicks( bool enable );
    int    widthScaleFactor() const;
    void   enableXGridLines( bool majorGridLines, bool minorGridLines );
    void   enableDepthGridLines( bool majorGridLines, bool minorGridLines );
    void   setMajorAndMinorTickIntervals( double majorTickInterval, double minorTickInterval );
    void   setAutoTickIntervalCounts( int maxMajorTickIntervalCount, int maxMinorTickIntervalCount );
    double getCurrentMajorTickInterval() const;
    double getCurrentMinorTickInterval() const;
    int    axisExtent( QwtPlot::Axis axis ) const;
    bool   frameIsInFrontOfThis( const QRect& frameGeometry );
    QPoint dragStartPosition() const;
    void   setDefaultStyleSheet();
    void   setStyleSheetForThisObject( const QString& content, const QString& state = "" );

    RimWellLogTrack* plotDefinition() const;

protected:
    bool  eventFilter( QObject* watched, QEvent* event ) override;
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    void  mousePressEvent( QMouseEvent* event ) override;
    void  mouseMoveEvent( QMouseEvent* event ) override;

private:
    void       setDefaults();
    void       selectClosestCurve( const QPoint& pos );
    static int defaultMinimumWidth();

private:
    caf::PdmPointer<RimWellLogTrack> m_plotTrackDefinition;
    QPoint                           m_dragStartPosition;
};
