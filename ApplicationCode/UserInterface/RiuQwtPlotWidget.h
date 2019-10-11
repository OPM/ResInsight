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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <QPointer>

#include <set>

class RiaPlotWindowRedrawScheduler;
class RimPlotInterface;

class QwtLegend;
class QwtPicker;
class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotMarker;
class QwtPlotPicker;

class QEvent;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuQwtPlotWidget : public QwtPlot
{
    Q_OBJECT

public:
    RiuQwtPlotWidget( RimPlotInterface* plotTrackDefinition, QWidget* parent = nullptr );
    ~RiuQwtPlotWidget() override;

    RimPlotInterface* plotDefinition() const;
    caf::PdmObject*   plotOwner() const;

    bool isChecked() const;

    int  fontSize() const;
    void setFontSize( int fontSize );

    void setEnabledAxes( const std::set<QwtPlot::Axis> enabledAxes );

    void setXTitle( const QString& title );
    void setYTitle( const QString& title );
    void setYTitleEnabled( bool enable );

    QwtInterval axisRange( QwtPlot::Axis axis );
    void        setXRange( double min, double max, QwtPlot::Axis axis = QwtPlot::xTop );
    void        setYRange( double min, double max );

    void setAxisInverted( QwtPlot::Axis axis );

    void setYAxisLabelsAndTicksEnabled( bool enable );

    void enableXGridLines( bool majorGridLines, bool minorGridLines );
    void enableYGridLines( bool majorGridLines, bool minorGridLines );

    void   setMajorAndMinorTickIntervals( double        majorTickInterval,
                                          double        minorTickInterval,
                                          double        minValue,
                                          double        maxValue,
                                          QwtPlot::Axis axis = QwtPlot::xTop );
    void   setAutoTickIntervalCounts( int           maxMajorTickIntervalCount,
                                      int           maxMinorTickIntervalCount,
                                      QwtPlot::Axis axis = QwtPlot::xTop );
    double getCurrentMajorTickInterval() const;
    double getCurrentMinorTickInterval() const;

    int axisExtent( QwtPlot::Axis axis ) const;

    bool   frameIsInFrontOfThis( const QRect& frameGeometry );
    QPoint dragStartPosition() const;
    void   setDefaultStyleSheet( bool includeHoverFrame = true );
    void   setStyleSheetForThisObject( const QString& content, const QString& state = "" );
    void   appendStyleSheetForThisObject( const QString& content, const QString& state = "" );
    int    widthScaleFactor() const;

    void scheduleReplot();
    void setSelected( bool selected );

protected:
    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    bool  eventFilter( QObject* watched, QEvent* event ) override;
    void  hideEvent( QHideEvent* event ) override;

    void applyYTitleToQwt();

    virtual void selectPoint( QwtPlotCurve* curve, int pointNumber );
    virtual void clearPointSelection();

    QString createHoverStyleSheet( const QString& borderType = "solid" );

private:
    void       setDefaults();
    void       selectPlotOwner( bool toggleItemInSelection = false );
    void       selectClosestCurve( const QPoint& pos, bool toggleItemInSelection = false );
    static int defaultMinimumWidth();
    void       replot() override;

    void highlightCurve( const QwtPlotCurve* closestCurve );
    void resetCurveHighlighting();
    void onAxisSelected( QwtScaleWidget* scale, bool toggleItemInSelection );

private:
    caf::PdmPointer<caf::PdmObject> m_plotOwner;
    QPoint                          m_clickPosition;
    QString                         m_yAxisTitle;
    bool                            m_yAxisTitleEnabled;
    QPointer<QwtPlotPicker>         m_plotPicker;

    struct CurveColors
    {
        QColor lineColor;
        QColor symbolColor;
        QColor symbolLineColor;
    };

    std::map<QwtPlotCurve*, CurveColors> m_originalCurveColors;
    std::map<QwtPlotCurve*, double>      m_originalZValues;

    friend class RiaPlotWindowRedrawScheduler;
};
