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

#include "RiuPlotWidget.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <QPointer>

#include <set>

class RiaPlotWindowRedrawScheduler;
class RimPlot;
class RiuPlotCurve;

class QEvent;
class QLabel;
class QPainter;
class QPaintDevice;
class QWheelEvent;

namespace QtCharts
{
class QValueAxis;
class QChart;
class QAbstractSeries;
class QAbstractAxis;
class QChartView;
}; // namespace QtCharts

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

    void setAxisFormat( RiaDefines::PlotAxis axis, const QString& format );

    void           setPlotTitle( const QString& plotTitle ) override;
    const QString& plotTitle() const;
    void           setPlotTitleEnabled( bool enabled );
    bool           plotTitleEnabled() const;
    void           setPlotTitleFontSize( int titleFontSize ) override;

    void setLegendFontSize( int fontSize ) override;
    void setInternalLegendVisible( bool visible ) override;
    void insertLegend( RiuPlotWidget::Legend ) override;
    void clearLegend() override;

    std::pair<double, double> axisRange( RiaDefines::PlotAxis axis ) const override;
    void                      setAxisRange( RiaDefines::PlotAxis axis, double min, double max ) override;

    void setAxisInverted( RiaDefines::PlotAxis axis, bool isInverted ) override;
    void setAxisLabelsAndTicksEnabled( RiaDefines::PlotAxis axis, bool enableLabels, bool enableTicks ) override;

    void enableGridLines( RiaDefines::PlotAxis axis, bool majorGridLines, bool minorGridLines ) override;

    void   setMajorAndMinorTickIntervals( RiaDefines::PlotAxis axis,
                                          double               majorTickInterval,
                                          double               minorTickInterval,
                                          double               minValue,
                                          double               maxValue ) override;
    void   setMajorAndMinorTickIntervalsAndRange( RiaDefines::PlotAxis axis,
                                                  double               majorTickInterval,
                                                  double               minorTickInterval,
                                                  double               minTickValue,
                                                  double               maxTickValue,
                                                  double               rangeMin,
                                                  double               rangeMax ) override;
    void   setAutoTickIntervalCounts( RiaDefines::PlotAxis axis,
                                      int                  maxMajorTickIntervalCount,
                                      int                  maxMinorTickIntervalCount ) override;
    double majorTickInterval( RiaDefines::PlotAxis axis ) const override;
    double minorTickInterval( RiaDefines::PlotAxis axis ) const override;

    void detachItems( RiuPlotWidget::PlotItemType plotItemType ) override;

    int axisExtent( RiaDefines::PlotAxis axis ) const override;

    QPoint dragStartPosition() const;

    void scheduleReplot();

    void updateLayout() override;

    void renderTo( QPainter* painter, const QRect& targetRect, double scaling ) override;
    void renderTo( QPaintDevice* painter, const QRect& targetRect ) override;
    int  overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

    void updateLegend() override;
    void updateAxes() override;

    RiuPlotCurve* createPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title, const QColor& color ) override;

    QtCharts::QChart* qtChart();

    void attach( RiuPlotCurve*              plotCurve,
                 QtCharts::QAbstractSeries* lineseries,
                 QtCharts::QAbstractSeries* scatterSeries,
                 RiaDefines::PlotAxis       xAxis,
                 RiaDefines::PlotAxis       yAxis );

    QtCharts::QAbstractSeries* getLineSeries( const RiuPlotCurve* plotCurve ) const;
    QtCharts::QAbstractSeries* getScatterSeries( const RiuPlotCurve* plotCurve ) const;

    void setXAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series );
    void setYAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series );

    const QColor& backgroundColor() const override;

    QWidget* getParentForOverlay() const override;

    std::pair<RiuPlotCurve*, int> findClosestCurve( const QPoint& pos, double& distanceToClick ) const override;

protected:
    void setAxis( RiaDefines::PlotAxis axis, QtCharts::QAbstractSeries* series );

    void resizeEvent( QResizeEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;
    void wheelEvent( QWheelEvent* event ) override;
    bool eventFilter( QObject* watched, QEvent* event ) override;

    void applyPlotTitleToPlot();
    void applyAxisTitleToPlot( RiaDefines::PlotAxis axis );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    virtual bool isZoomerActive() const;
    virtual void endZoomOperations();

    void                     rescaleAxis( RiaDefines::PlotAxis axis );
    QtCharts::QAbstractAxis* plotAxis( RiaDefines::PlotAxis axis ) const;
    Qt::Orientation          orientation( RiaDefines::PlotAxis axis ) const;

    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;

signals:
    void plotZoomed();

private slots:
    void axisRangeChanged();

private:
    void                 addAxis( RiaDefines::PlotAxis plotAxis, bool isEnabled, bool isAutoScale );
    static Qt::Alignment mapPlotAxisToQtAlignment( RiaDefines::PlotAxis axis );

    static int defaultMinimumWidth();
    void       replot() override;

    QPointer<QtCharts::QChartView> m_viewer;

    std::map<RiaDefines::PlotAxis, QtCharts::QAbstractAxis*> m_axes;
    std::map<RiaDefines::PlotAxis, bool>                     m_axesEnabled;
    std::map<RiaDefines::PlotAxis, bool>                     m_axesAutoScale;

    std::map<const RiuPlotCurve*, QtCharts::QAbstractSeries*> m_lineSeriesMap;
    std::map<const RiuPlotCurve*, QtCharts::QAbstractSeries*> m_scatterSeriesMap;
};
