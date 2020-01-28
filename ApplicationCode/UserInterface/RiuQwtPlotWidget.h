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

#include "RiuInterfaceToViewWindow.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"
#include "cafUiStyleSheet.h"

#include "qwt_plot.h"

#include <QPointer>

#include <set>

class RiaPlotWindowRedrawScheduler;
class RimPlot;
class RiuDraggableOverlayFrame;

class QwtLegend;
class QwtPicker;
class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotMarker;
class QwtPlotPicker;

class QEvent;
class QLabel;
class QPainter;
class QPaintDevice;
class QWheelEvent;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuQwtPlotWidget : public QwtPlot, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    RiuQwtPlotWidget( RimPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuQwtPlotWidget() override;

    RimPlot* plotDefinition();

    bool isChecked() const;

    int colSpan() const;
    int rowSpan() const;

    int  axisTitleFontSize( QwtPlot::Axis axis ) const;
    int  axisValueFontSize( QwtPlot::Axis axis ) const;
    void setAxisFontsAndAlignment( QwtPlot::Axis,
                                   int               titleFontSize,
                                   int               valueFontSize,
                                   bool              titleBold = false,
                                   Qt::AlignmentFlag alignment = Qt::AlignRight );

    void setAxisTitleText( QwtPlot::Axis axis, const QString& title );
    void setAxisTitleEnabled( QwtPlot::Axis axis, bool enable );

    void           setPlotTitle( const QString& plotTitle );
    const QString& plotTitle() const;
    void           setPlotTitleEnabled( bool enabled );
    bool           plotTitleEnabled() const;

    QwtInterval axisRange( QwtPlot::Axis axis );
    void        setAxisRange( QwtPlot::Axis axis, double min, double max );

    void setAxisInverted( QwtPlot::Axis axis );
    void setAxisLabelsAndTicksEnabled( QwtPlot::Axis axis, bool enable );

    void enableGridLines( QwtPlot::Axis axis, bool majorGridLines, bool minorGridLines );

    void setMajorAndMinorTickIntervals( QwtPlot::Axis axis,
                                        double        majorTickInterval,
                                        double        minorTickInterval,
                                        double        minValue,
                                        double        maxValue );
    void setAutoTickIntervalCounts( QwtPlot::Axis axis, int maxMajorTickIntervalCount, int maxMinorTickIntervalCount );
    double majorTickInterval( QwtPlot::Axis axis ) const;
    double minorTickInterval( QwtPlot::Axis axis ) const;

    int axisExtent( QwtPlot::Axis axis ) const;

    bool   frameIsInFrontOfThis( const QRect& frameGeometry );
    QPoint dragStartPosition() const;

    void scheduleReplot();
    void stashWidgetStates();
    void restoreWidgetStates();
    void setWidgetState( const QString& widgetState );

    void addOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void removeOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void updateLayout() override;

    void renderTo( QPainter* painter, const QRect& targetRect, double scaling );
    void renderTo( QPaintDevice* painter, const QRect& targetRect );
    int  overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

signals:
    void plotSelected( bool toggleSelection );
    void axisSelected( int axisId, bool toggleSelection );
    void curveSelected( QwtPlotCurve* curve, bool toggleSelection );
    void onKeyPressEvent( QKeyEvent* event );
    void onWheelEvent( QWheelEvent* event );

protected:
    bool eventFilter( QObject* watched, QEvent* event ) override;
    void hideEvent( QHideEvent* event ) override;
    void showEvent( QShowEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;

    void applyPlotTitleToQwt();
    void applyAxisTitleToQwt( QwtPlot::Axis axis );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    virtual void selectPoint( QwtPlotCurve* curve, int pointNumber );
    virtual void clearPointSelection();
    virtual bool isZoomerActive() const;
    virtual void endZoomOperations();

private:
    void       selectClosestCurve( const QPoint& pos, bool toggleItemInSelection = false );
    static int defaultMinimumWidth();
    void       replot() override;

    void highlightCurve( const QwtPlotCurve* closestCurve );
    void resetCurveHighlighting();
    void onAxisSelected( QwtScaleWidget* scale, bool toggleItemInSelection );

    caf::UiStyleSheet createPlotStyleSheet() const;
    caf::UiStyleSheet createCanvasStyleSheet() const;

    void updateOverlayFrameLayout();

private:
    caf::PdmPointer<RimPlot>         m_plotDefinition;
    QPoint                           m_clickPosition;
    std::map<QwtPlot::Axis, QString> m_axisTitles;
    std::map<QwtPlot::Axis, bool>    m_axisTitlesEnabled;
    QPointer<QwtPlotPicker>          m_plotPicker;
    const int                        m_overlayMargins;
    QString                          m_plotTitle;
    bool                             m_plotTitleEnabled;

    QList<QPointer<RiuDraggableOverlayFrame>> m_overlayFrames;

    struct CurveColors
    {
        QColor lineColor;
        QColor symbolColor;
        QColor symbolLineColor;
    };

    std::map<QwtPlotCurve*, CurveColors> m_originalCurveColors;
    std::map<QwtPlotCurve*, double>      m_originalZValues;

    caf::UiStyleSheet m_plotStyleSheet;
    caf::UiStyleSheet m_canvasStyleSheet;

    friend class RiaPlotWindowRedrawScheduler;
};
