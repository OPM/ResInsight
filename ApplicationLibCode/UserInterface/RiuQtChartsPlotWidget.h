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
#include "RiaPlotDefines.h"

#include "RiuInterfaceToViewWindow.h"
#include "RiuPlotWidget.h"
#include "RiuQtChartView.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <QPointer>

#include <set>

class RiaPlotWindowRedrawScheduler;
class RimPlot;
class RiuDraggableOverlayFrame;
class RiuPlotCurve;

class QEvent;
class QLabel;
class QPainter;
class QPaintDevice;
class QWheelEvent;

namespace QtCharts
{
class QValueAxis;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RiuQtChartsPlotWidget : public RiuPlotWidget
{
    Q_OBJECT

public:
    RiuQtChartsPlotWidget( RimPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuQtChartsPlotWidget() override;

    int  axisTitleFontSize( RiaDefines::PlotAxis axis ) const override;
    int  axisValueFontSize( RiaDefines::PlotAxis axis ) const override;
    void setAxisFontsAndAlignment( RiaDefines::PlotAxis,
                                   int  titleFontSize,
                                   int  valueFontSize,
                                   bool titleBold = false,
                                   int  alignment = (int)Qt::AlignCenter ) override;
    void setAxesFontsAndAlignment( int  titleFontSize,
                                   int  valueFontSize,
                                   bool titleBold = false,
                                   int  alignment = (int)Qt::AlignCenter ) override;

    void enableAxis( RiaDefines::PlotAxis axis, bool isEnabled ) override;
    bool axisEnabled( RiaDefines::PlotAxis axis ) const override;

    void setAxisScale( RiaDefines::PlotAxis axis, double min, double max ) override;
    void setAxisAutoScale( RiaDefines::PlotAxis axis, bool enable ) override;

    void setAxisMaxMinor( RiaDefines::PlotAxis axis, int maxMinor ) override;
    void setAxisMaxMajor( RiaDefines::PlotAxis axis, int maxMajor ) override;

    RiuPlotWidget::AxisScaleType axisScaleType( RiaDefines::PlotAxis axis ) const override;
    void setAxisScaleType( RiaDefines::PlotAxis axis, RiuPlotWidget::AxisScaleType axisScaleType ) override;

    void setAxisTitleText( RiaDefines::PlotAxis axis, const QString& title ) override;
    void setAxisTitleEnabled( RiaDefines::PlotAxis axis, bool enable ) override;

    void           setPlotTitle( const QString& plotTitle ) override;
    const QString& plotTitle() const;
    void           setPlotTitleEnabled( bool enabled );
    bool           plotTitleEnabled() const;
    void           setPlotTitleFontSize( int titleFontSize ) override;

    void setLegendFontSize( int fontSize ) override;
    void setInternalLegendVisible( bool visible );
    void insertLegend( RiuPlotWidget::Legend ) override;
    void clearLegend() override;

    std::pair<double, double> axisRange( RiaDefines::PlotAxis axis ) const override;
    void                      setAxisRange( RiaDefines::PlotAxis axis, double min, double max ) override;

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

    void detachItems( RiuPlotWidget::PlotItemType plotItemType ) override;

    int axisExtent( RiaDefines::PlotAxis axis ) const;

    bool   frameIsInFrontOfThis( const QRect& frameGeometry );
    QPoint dragStartPosition() const;

    void scheduleReplot();

    void addOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void removeOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void updateLayout();

    void renderTo( QPainter* painter, const QRect& targetRect, double scaling ) override;
    void renderTo( QPaintDevice* painter, const QRect& targetRect ) override;
    int  overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

    void removeEventFilter() override;

    void updateLegend();
    void updateAxes();

    RiuPlotCurve* createPlotCurve( const QString& title, const QColor& color ) override;

    // signals:
    //     void plotSelected( bool toggleSelection );
    //     void axisSelected( int axisId, bool toggleSelection );
    //     void onKeyPressEvent( QKeyEvent* event );
    //     void onWheelEvent( QWheelEvent* event );
    //     void plotZoomed();

    QtCharts::QChart* qtChart();

    void attach( QtCharts::QAbstractSeries* series, RiaDefines::PlotAxis xAxis, RiaDefines::PlotAxis yAxis );

    void setXAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series );
    void setYAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series );

protected:
    void setAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series );

    bool eventFilter( QObject* watched, QEvent* event ) override;
    void hideEvent( QHideEvent* event ) override;
    void showEvent( QShowEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;

    void applyPlotTitleToPlot();
    void applyPlotTitleToQtCharts();
    void applyAxisTitleToPlot( RiaDefines::PlotAxis axis );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    virtual bool isZoomerActive() const;
    virtual void endZoomOperations();

    void                     rescaleAxis( RiaDefines::PlotAxis axis );
    QtCharts::QAbstractAxis* plotAxis( RiaDefines::PlotAxis axis ) const;
    Qt::Orientation          orientation( RiaDefines::PlotAxis axis ) const;

private:
    void       selectClosestPlotItem( const QPoint& pos, bool toggleItemInSelection = false );
    static int defaultMinimumWidth();
    void       replot() override;

    // void highlightPlotItem( const QtChartsPlotItem* closestItem );
    // void resetPlotItemHighlighting();
    // void onAxisSelected( QtChartsScaleWidget* scale, bool toggleItemInSelection );
    // void recalculateAxisExtents( RiaDefines::PlotAxis axis );

    void updateOverlayFrameLayout();

private:
    // std::map<QtChartsPlotCurve*, CurveColors> m_originalCurveColors;
    // std::map<QtChartsPlotCurve*, double>      m_originalZValues;

    // QPointer<QtChartsPlot> m_plot;

    QPointer<QtCharts::QChartView> m_viewer;

    std::map<RiaDefines::PlotAxis, QtCharts::QAbstractAxis*> m_axes;

    friend class RiaPlotWindowRedrawScheduler;
};
