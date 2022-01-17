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

class RiaPlotWindowRedrawScheduler;
class RimPlot;
class RimPlotCurve;

class RiuPlotCurve;
class RiuPlotItem;

class QwtPlot;
class QwtLegend;
class QwtPicker;
class QwtPlotCurve;
class QwtPlotGrid;
class QwtPlotItem;
class QwtPlotMarker;
class QwtScaleWidget;

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
class RiuQwtPlotWidget : public RiuPlotWidget
{
    Q_OBJECT

public:
    RiuQwtPlotWidget( RimPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuQwtPlotWidget() override;

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

    int axisExtent( RiaDefines::PlotAxis axis ) const override;

    QPoint dragStartPosition() const;

    void scheduleReplot();

    void updateLayout() override;

    void renderTo( QPainter* painter, const QRect& targetRect, double scaling ) override;
    void renderTo( QPaintDevice* painter, const QRect& targetRect ) override;
    int  overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

    QwtPlot* qwtPlot() const;

    void removeEventFilter() override;

    void updateLegend() override;
    void updateAxes() override;

    RiuPlotCurve* createPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title, const QColor& color ) override;

    void detachItems( RiuPlotWidget::PlotItemType plotItemType ) override;

    void findClosestPlotItem( const QPoint& pos,
                              QwtPlotItem** closestItem,
                              int*          closestCurvePoint,
                              double*       distanceFromClick ) const;

    const QColor& backgroundColor() const override;

    QWidget* getParentForOverlay() const override;

    std::pair<RiuPlotCurve*, int> findClosestCurve( const QPoint& pos, double& distanceToClick ) const override;

signals:
    void plotSelected( bool toggleSelection );
    void axisSelected( int axisId, bool toggleSelection );
    void plotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggleSelection, int sampleIndex );
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

private:
    void       selectClosestPlotItem( const QPoint& pos, bool toggleItemInSelection = false );
    static int defaultMinimumWidth();
    void       replot() override;

    void highlightPlotItem( const QwtPlotItem* closestItem );
    void resetPlotItemHighlighting();
    void onAxisSelected( QwtScaleWidget* scale, bool toggleItemInSelection );
    void recalculateAxisExtents( RiaDefines::PlotAxis axis );

private:
    struct CurveColors
    {
        QColor lineColor;
        QColor symbolColor;
        QColor symbolLineColor;
    };

    std::map<QwtPlotCurve*, CurveColors> m_originalCurveColors;
    std::map<QwtPlotCurve*, double>      m_originalZValues;

    QPointer<QwtPlot> m_plot;
};
