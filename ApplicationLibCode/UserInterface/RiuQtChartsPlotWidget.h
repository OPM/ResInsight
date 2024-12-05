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

#include "RiaDateTimeDefines.h"
#include "RiaDefines.h"
#include "RiaPlotDefines.h"

#include "RiuPlotWidget.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include "qwt_legend_data.h"

#include <QPointer>

#include <set>

class RiaPlotWindowRedrawScheduler;
class RimPlot;
class RiuPlotCurve;
class RiuQtChartsPlotCurve;
class RiuQtChartsToolTip;
class RiuPlotCurveSymbol;
class RiuPlotCurveInfoTextProvider;

class QEvent;
class QLabel;
class QPainter;
class QPaintDevice;
class QWheelEvent;
class RiuQwtDateScaleWrapper;

class QValueAxis;
class QChart;
class QAbstractSeries;
class QAbstractAxis;
class QChartView;
class QCategoryAxis;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuQtChartsPlotWidget : public RiuPlotWidget
{
    Q_OBJECT

public:
    RiuQtChartsPlotWidget( RimPlot* plotDefinition, QWidget* parent = nullptr, RiuPlotCurveInfoTextProvider* plotCurveNameProvider = nullptr );
    ~RiuQtChartsPlotWidget() override;

    int  axisTitleFontSize( RiuPlotAxis axis ) const override;
    int  axisValueFontSize( RiuPlotAxis axis ) const override;
    void setAxisFontsAndAlignment( RiuPlotAxis,
                                   int  titleFontSize,
                                   int  valueFontSize,
                                   bool titleBold = false,
                                   int  alignment = (int)Qt::AlignCenter ) override;
    void setAxesFontsAndAlignment( int titleFontSize, int valueFontSize, bool titleBold = false, int alignment = (int)Qt::AlignCenter ) override;

    void enableAxisNumberLabels( RiuPlotAxis axis, bool isEnabled ) override;
    void enableAxis( RiuPlotAxis axis, bool isEnabled ) override;
    bool axisEnabled( RiuPlotAxis axis ) const override;

    void setAxisScale( RiuPlotAxis axis, double min, double max ) override;
    void setAxisAutoScale( RiuPlotAxis axis, bool enable ) override;

    void setAxisMaxMinor( RiuPlotAxis axis, int maxMinor ) override;
    void setAxisMaxMajor( RiuPlotAxis axis, int maxMajor ) override;

    RiuPlotWidget::AxisScaleType axisScaleType( RiuPlotAxis axis ) const override;
    void                         setAxisScaleType( RiuPlotAxis axis, RiuPlotWidget::AxisScaleType axisScaleType ) override;

    void setAxisTitleText( RiuPlotAxis axis, const QString& title ) override;
    void setAxisTitleEnabled( RiuPlotAxis axis, bool enable ) override;

    void setAxisFormat( RiuPlotAxis axis, const QString& format );

    void pruneAxes( const std::set<RiuPlotAxis>& usedAxis ) override;
    void moveAxis( RiuPlotAxis oldAxis, RiuPlotAxis newAxis ) override;

    RiuPlotAxis createNextPlotAxis( RiaDefines::PlotAxis axis ) override;
    bool        isMultiAxisSupported() const override;

    void           setPlotTitle( const QString& plotTitle ) override;
    const QString& plotTitle() const;
    void           setPlotTitleEnabled( bool enabled ) override;
    bool           plotTitleEnabled() const override;
    void           setPlotTitleFontSize( int titleFontSize ) override;

    void setLegendFontSize( int fontSize ) override;
    void insertLegend( RiuPlotWidget::Legend ) override;
    void clearLegend() override;

    std::pair<double, double> axisRange( RiuPlotAxis axis ) const override;
    void                      setAxisRange( RiuPlotAxis axis, double min, double max ) override;

    void setAxisInverted( RiuPlotAxis axis, bool isInverted ) override;
    void setAxisLabelsAndTicksEnabled( RiuPlotAxis axis, bool enableLabels, bool enableTicks ) override;

    void enableGridLines( RiuPlotAxis axis, bool majorGridLines, bool minorGridLines ) override;

    void setMajorTicksList( RiuPlotAxis axis, const QList<double>& majorTicks, double minValue, double maxValue ) override;
    void setMajorAndMinorTickIntervals( RiuPlotAxis axis, double majorTickInterval, double minorTickInterval, double minValue, double maxValue ) override;
    void   setMajorAndMinorTickIntervalsAndRange( RiuPlotAxis axis,
                                                  double      majorTickInterval,
                                                  double      minorTickInterval,
                                                  double      minTickValue,
                                                  double      maxTickValue,
                                                  double      rangeMin,
                                                  double      rangeMax ) override;
    void   setAutoTickIntervalCounts( RiuPlotAxis axis, int maxMajorTickIntervalCount, int maxMinorTickIntervalCount ) override;
    double majorTickInterval( RiuPlotAxis axis ) const override;
    double minorTickInterval( RiuPlotAxis axis ) const override;

    void detachItems( RiuPlotWidget::PlotItemType plotItemType ) override;

    int axisExtent( RiuPlotAxis axis ) const override;

    void ensureAxisIsCreated( RiuPlotAxis axis ) override;

    QPoint dragStartPosition() const;

    void scheduleReplot();

    void updateLayout() override;

    void renderTo( QPainter* painter, const QRect& targetRect, double scaling ) override;
    void renderTo( QPaintDevice* painter, const QRect& targetRect ) override;
    int  overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

    void updateLegend() override;
    void updateAxes() override;

    RiuPlotCurve* createPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title ) override;

    QChart* qtChart();

    void attach( RiuPlotCurve*    plotCurve,
                 QAbstractSeries* lineSeries,
                 QAbstractSeries* areaSeries,
                 QAbstractSeries* scatterSeries,
                 RiuPlotAxis      xAxis,
                 RiuPlotAxis      yAxis );
    void detach( RiuPlotCurve* plotCurve );

    QAbstractSeries* getLineSeries( const RiuPlotCurve* plotCurve ) const;
    QAbstractSeries* getAreaSeries( const RiuPlotCurve* plotCurve ) const;
    QAbstractSeries* getScatterSeries( const RiuPlotCurve* plotCurve ) const;

    void setXAxis( RiuPlotAxis axis, QAbstractSeries* series, RiuQtChartsPlotCurve* plotCurve );
    void setYAxis( RiuPlotAxis axis, QAbstractSeries* series, RiuQtChartsPlotCurve* plotCurve );

    const QColor& backgroundColor() const override;

    QWidget* getParentForOverlay() const override;

    std::pair<RiuPlotCurve*, int> findClosestCurve( const QPoint& pos, double& distanceToClick ) const override;

    void updateZoomDependentCurveProperties() override;

    void setFormatStrings( const QString&                   dateFormat,
                           const QString&                   timeFormat,
                           RiaDefines::DateFormatComponents dateComponents,
                           RiaDefines::TimeFormatComponents timeComponents );

protected:
    void attachSeriesToAxis( RiuPlotAxis axis, QAbstractSeries* series, RiuQtChartsPlotCurve* plotCurve );

    void resizeEvent( QResizeEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;
    void wheelEvent( QWheelEvent* event ) override;
    bool eventFilter( QObject* watched, QEvent* event ) override;

    void applyPlotTitleToPlot();
    void applyAxisTitleToPlot( RiuPlotAxis axis );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    virtual bool isZoomerActive() const;
    virtual void endZoomOperations();

    void            rescaleAxis( RiuPlotAxis axis );
    QAbstractAxis*  plotAxis( RiuPlotAxis axis ) const;
    Qt::Orientation orientation( RiaDefines::PlotAxis axis ) const;

    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;

signals:
    void plotZoomed();
    void legendDataChanged( const QList<QwtLegendData>& data );

private slots:
    void axisRangeChanged();
    void tooltip( const QPointF& point, bool state );

private:
    void addAxis( RiuPlotAxis plotAxis, bool isEnabled, bool isAutoScale );
    void deleteAxis( RiuPlotAxis plotAxis );

    static Qt::Alignment mapPlotAxisToQtAlignment( RiaDefines::PlotAxis axis );

    static int defaultMinimumWidth();
    void       replot() override;

    QCategoryAxis* categoryAxis();

    QString createNameFromSeries( QAbstractSeries* series ) const;

private:
    QPointer<QChartView> m_viewer;

    std::map<RiuPlotAxis, QAbstractAxis*> m_axes;
    std::map<RiuPlotAxis, bool>           m_axesEnabled;
    std::map<RiuPlotAxis, bool>           m_axesAutoScale;

    std::map<RiuPlotCurve*, QAbstractSeries*> m_lineSeriesMap;
    std::map<RiuPlotCurve*, QAbstractSeries*> m_areaSeriesMap;
    std::map<RiuPlotCurve*, QAbstractSeries*> m_scatterSeriesMap;

    RiuQwtDateScaleWrapper*       m_dateScaleWrapper;
    RiuQtChartsToolTip*           m_toolTip;
    RiuPlotCurveInfoTextProvider* m_plotCurveNameProvider;
};
