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

#include "RiaDefines.h"
#include "RiuInterfaceToViewWindow.h"

#include "RiaPlotDefines.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "qwt_plot.h"

#include <QPointer>

#include <set>

class RiaPlotWindowRedrawScheduler;
class RimPlot;
class RiuDraggableOverlayFrame;
class RiuPlotCurve;

class QwtLegend;
class QwtPicker;
class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotItem;
class QwtPlotMarker;

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
class RiuQwtPlotWidget : public QWidget, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    enum class AxisScaleType
    {
        LINEAR,
        LOGARITHMIC,
        DATE
    };

    RiuQwtPlotWidget( RimPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuQwtPlotWidget() override;

    RimPlot* plotDefinition();

    bool isChecked() const;

    int colSpan() const;
    int rowSpan() const;

    int  axisTitleFontSize( RiaDefines::PlotAxis axis ) const;
    int  axisValueFontSize( RiaDefines::PlotAxis axis ) const;
    void setAxisFontsAndAlignment( RiaDefines::PlotAxis,
                                   int  titleFontSize,
                                   int  valueFontSize,
                                   bool titleBold = false,
                                   int  alignment = (int)Qt::AlignCenter );
    void setAxesFontsAndAlignment( int  titleFontSize,
                                   int  valueFontSize,
                                   bool titleBold = false,
                                   int  alignment = (int)Qt::AlignCenter );

    void enableAxis( RiaDefines::PlotAxis axis, bool isEnabled );
    bool axisEnabled( RiaDefines::PlotAxis axis ) const;

    void setAxisScale( RiaDefines::PlotAxis axis, double min, double max );
    void setAxisAutoScale( RiaDefines::PlotAxis axis, bool enable );

    void setAxisMaxMinor( RiaDefines::PlotAxis axis, int maxMinor );
    void setAxisMaxMajor( RiaDefines::PlotAxis axis, int maxMajor );

    RiuQwtPlotWidget::AxisScaleType axisScaleType( RiaDefines::PlotAxis axis ) const;
    void setAxisScaleType( RiaDefines::PlotAxis axis, RiuQwtPlotWidget::AxisScaleType axisScaleType );

    void setAxisTitleText( RiaDefines::PlotAxis axis, const QString& title );
    void setAxisTitleEnabled( RiaDefines::PlotAxis axis, bool enable );

    void           setPlotTitle( const QString& plotTitle );
    const QString& plotTitle() const;
    void           setPlotTitleEnabled( bool enabled );
    bool           plotTitleEnabled() const;
    void           setPlotTitleFontSize( int titleFontSize );

    void setLegendFontSize( int fontSize );
    void setInternalLegendVisible( bool visible );

    QwtInterval axisRange( RiaDefines::PlotAxis axis ) const;
    void        setAxisRange( RiaDefines::PlotAxis axis, double min, double max );

    void setAxisInverted( RiaDefines::PlotAxis axis, bool isInverted );
    void setAxisLabelsAndTicksEnabled( RiaDefines::PlotAxis axis, bool enableLabels, bool enableTicks );

    void enableGridLines( RiaDefines::PlotAxis axis, bool majorGridLines, bool minorGridLines );

    void setMajorAndMinorTickIntervals( RiaDefines::PlotAxis axis,
                                        double               majorTickInterval,
                                        double               minorTickInterval,
                                        double               minValue,
                                        double               maxValue );
    void setMajorAndMinorTickIntervalsAndRange( RiaDefines::PlotAxis axis,
                                                double               majorTickInterval,
                                                double               minorTickInterval,
                                                double               minTickValue,
                                                double               maxTickValue,
                                                double               rangeMin,
                                                double               rangeMax );
    void setAutoTickIntervalCounts( RiaDefines::PlotAxis axis, int maxMajorTickIntervalCount, int maxMinorTickIntervalCount );
    double majorTickInterval( RiaDefines::PlotAxis axis ) const;
    double minorTickInterval( RiaDefines::PlotAxis axis ) const;

    int axisExtent( RiaDefines::PlotAxis axis ) const;

    bool   frameIsInFrontOfThis( const QRect& frameGeometry );
    QPoint dragStartPosition() const;

    void scheduleReplot();

    void addOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void removeOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void updateLayout();

    void renderTo( QPainter* painter, const QRect& targetRect, double scaling );
    void renderTo( QPaintDevice* painter, const QRect& targetRect );
    int  overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

    QwtPlot* qwtPlot() const;

    void removeEventFilter();

    void updateLegend();
    void updateAxes();

    RiuPlotCurve* createPlotCurve( const QString& title, const QColor& color );

signals:
    void plotSelected( bool toggleSelection );
    void axisSelected( int axisId, bool toggleSelection );
    void plotItemSelected( QwtPlotItem* plotItem, bool toggleSelection, int sampleIndex );
    void onKeyPressEvent( QKeyEvent* event );
    void onWheelEvent( QWheelEvent* event );
    void plotZoomed();

protected:
    bool eventFilter( QObject* watched, QEvent* event ) override;
    void hideEvent( QHideEvent* event ) override;
    void showEvent( QShowEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;

    void applyPlotTitleToQwt();
    void applyAxisTitleToQwt( RiaDefines::PlotAxis axis );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    virtual bool isZoomerActive() const;
    virtual void endZoomOperations();

    void findClosestPlotItem( const QPoint& pos,
                              QwtPlotItem** closestItem,
                              int*          closestCurvePoint,
                              double*       distanceFromClick ) const;

private:
    void       selectClosestPlotItem( const QPoint& pos, bool toggleItemInSelection = false );
    static int defaultMinimumWidth();
    void       replot();

    void highlightPlotItem( const QwtPlotItem* closestItem );
    void resetPlotItemHighlighting();
    void onAxisSelected( QwtScaleWidget* scale, bool toggleItemInSelection );
    void recalculateAxisExtents( RiaDefines::PlotAxis axis );

    void updateOverlayFrameLayout();

private:
    caf::PdmPointer<RimPlot>                m_plotDefinition;
    QPoint                                  m_clickPosition;
    std::map<RiaDefines::PlotAxis, QString> m_axisTitles;
    std::map<RiaDefines::PlotAxis, bool>    m_axisTitlesEnabled;
    const int                               m_overlayMargins;
    QString                                 m_plotTitle;
    bool                                    m_plotTitleEnabled;

    QList<QPointer<RiuDraggableOverlayFrame>> m_overlayFrames;

    struct CurveColors
    {
        QColor lineColor;
        QColor symbolColor;
        QColor symbolLineColor;
    };

    std::map<QwtPlotCurve*, CurveColors> m_originalCurveColors;
    std::map<QwtPlotCurve*, double>      m_originalZValues;

    QPointer<QwtPlot> m_plot;

    friend class RiaPlotWindowRedrawScheduler;
};
