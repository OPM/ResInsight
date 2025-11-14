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

#include "qwt_axis_id.h"

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

    int  axisTitleFontSize( RiuPlotAxis axis ) const override;
    int  axisValueFontSize( RiuPlotAxis axis ) const override;
    void setAxisFontsAndAlignment( RiuPlotAxis,
                                   int  titleFontSize,
                                   int  valueFontSize,
                                   bool titleBold = false,
                                   int  alignment = (int)Qt::AlignCenter ) override;
    void setAxesFontsAndAlignment( int titleFontSize, int valueFontSize, bool titleBold = false, int alignment = (int)Qt::AlignCenter ) override;

    void enableAxis( RiuPlotAxis axis, bool isEnabled ) override;
    void enableAxisNumberLabels( RiuPlotAxis axis, bool isEnabled ) override;
    bool axisEnabled( RiuPlotAxis axis ) const override;

    void setAxisScale( RiuPlotAxis axis, double min, double max ) override;
    void setAxisAutoScale( RiuPlotAxis axis, bool enable ) override;

    void setAxisMaxMinor( RiuPlotAxis axis, int maxMinor ) override;
    void setAxisMaxMajor( RiuPlotAxis axis, int maxMajor ) override;

    RiuPlotWidget::AxisScaleType axisScaleType( RiuPlotAxis axis ) const override;
    void                         setAxisScaleType( RiuPlotAxis axis, RiuPlotWidget::AxisScaleType axisScaleType ) override;

    void setAxisTitleText( RiuPlotAxis axis, const QString& title ) override;
    void setAxisTitleEnabled( RiuPlotAxis axis, bool enable ) override;

    void        moveAxis( RiuPlotAxis oldAxis, RiuPlotAxis newAxis ) override;
    void        pruneAxes( const std::set<RiuPlotAxis>& usedAxes ) override;
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

    int axisExtent( RiuPlotAxis axis ) const override;

    void ensureAxisIsCreated( RiuPlotAxis axis ) override;

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

    RiuPlotCurve* createPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title ) override;

    void detachItems( RiuPlotWidget::PlotItemType plotItemType ) override;

    const QColor& backgroundColor() const override;

    QWidget* getParentForOverlay() const override;

    std::pair<RiuPlotCurve*, int> findClosestCurve( const QPoint& pos, double& distanceToClick ) const override;

    QwtAxisId toQwtPlotAxis( RiuPlotAxis axis ) const;

    void                                       highlightPlotItem( const QwtPlotItem* plotItem );
    void                                       highlightCurvesUpdateOrder( const std::vector<RimPlotCurve*>& curves );
    void                                       resetPlotItemHighlighting( bool doUpdateCurveOrder = true );
    std::vector<caf::PdmPointer<RimPlotCurve>> highlightedCurves() const;

    void replot() override;

public slots:
    void onLegendClicked( const QVariant& itemInfo, int index );

signals:
    void plotSelected( bool toggleSelection );
    void axisSelected( RiuPlotAxis axisId, bool toggleSelection );
    void plotItemSelected( std::shared_ptr<RiuPlotItem> plotItem, bool toggleSelection, int sampleIndex );
    void onKeyPressEvent( QKeyEvent* event );
    void onWheelEvent( QWheelEvent* event );
    void plotZoomed();

protected:
    bool eventFilter( QObject* watched, QEvent* event ) override;
    void hideEvent( QHideEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;
    void keyPressEvent( QKeyEvent* event ) override;

    void applyPlotTitleToQwt();
    void applyAxisTitleToQwt( RiuPlotAxis axis );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;

    virtual bool isZoomerActive() const;
    virtual void endZoomOperations();

    void setAxisScaleType( QwtAxisId axis, RiuQwtPlotWidget::AxisScaleType axisScaleType );
    void setAxisScale( QwtAxisId axis, double min, double max );

    RiuPlotAxis findPlotAxisForQwtAxis( const QwtAxisId& qwtAxisId ) const;

    virtual void onMouseMoveEvent( QMouseEvent* event );

private:
    void selectClosestPlotItem( const QPoint& pos, bool toggleItemInSelection = false );
    void findClosestPlotItem( const QPoint& pos, QwtPlotItem** closestItem, int* closestCurveSampleIndex, double* distanceFromClick ) const;

    static int defaultMinimumWidth();

    void highlightPlotAxes( QwtAxisId axisIdX, QwtAxisId axisIdY );
    void highlightPlotItemsForQwtAxis( QwtAxisId axisId );
    void highlightPlotCurves( const std::vector<RimPlotCurve*>& curves );
    void highlightPlotShapeItems( const std::set<const QwtPlotItem*>& closestItems );
    void resetPlotCurveHighlighting();
    void resetPlotShapeItemHighlighting();
    void resetPlotAxisHighlighting();
    void onAxisSelected( QwtScaleWidget* scale, bool toggleItemInSelection );
    void recalculateAxisExtents( RiuPlotAxis axis );

    static int highlightItemWidthAdjustment();

    void updateCurveOrder();

private:
    struct CurveProperties
    {
        QColor lineColor;
        QColor symbolColor;
        QColor symbolLineColor;
        int    lineWidth;
    };

    std::map<QwtPlotCurve*, double>            m_originalZValues;
    std::vector<caf::PdmPointer<RimPlotCurve>> m_highlightedCurves;
    std::map<RiuPlotAxis, QwtAxisId>           m_axisMapping;

    QPointer<QwtPlot> m_plot;

    int m_titleRenderingFlags;
    int m_titleFontSize;
};
