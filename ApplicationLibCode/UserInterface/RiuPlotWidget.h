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

#include "RiaPlotDefines.h"
#include "RiuInterfaceToViewWindow.h"
#include "RiuPlotAxis.h"

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <QPointer>
#include <QWidget>

class RiaPlotWindowRedrawScheduler;
class RimPlot;
class RimPlotCurve;

class RiuDraggableOverlayFrame;
class RiuPlotCurve;

class QPainter;
class QPaintDevice;

//==================================================================================================
//
//
//
//==================================================================================================
class RiuPlotWidget : public QWidget, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    enum class AxisScaleType
    {
        LINEAR,
        LOGARITHMIC,
        DATE
    };

    enum class Legend
    {
        BOTTOM,
        TOP,
        LEFT,
        RIGHT
    };

    enum class PlotItemType
    {
        CURVE,
        LEGEND
    };

    RiuPlotWidget( RimPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuPlotWidget() override;

    RimPlot* plotDefinition();

    bool isChecked() const;

    int colSpan() const;
    int rowSpan() const;

    virtual int  axisTitleFontSize( RiuPlotAxis axis ) const                      = 0;
    virtual int  axisValueFontSize( RiuPlotAxis axis ) const                      = 0;
    virtual void setAxisFontsAndAlignment( RiuPlotAxis,
                                           int  titleFontSize,
                                           int  valueFontSize,
                                           bool titleBold = false,
                                           int  alignment = (int)Qt::AlignCenter ) = 0;
    virtual void setAxesFontsAndAlignment( int  titleFontSize,
                                           int  valueFontSize,
                                           bool titleBold = false,
                                           int  alignment = (int)Qt::AlignCenter ) = 0;

    virtual void enableAxisNumberLabels( RiuPlotAxis axis, bool isEnabled ) = 0;
    virtual void enableAxis( RiuPlotAxis axis, bool isEnabled )             = 0;
    virtual bool axisEnabled( RiuPlotAxis axis ) const                      = 0;

    virtual void setAxisScale( RiuPlotAxis axis, double min, double max ) = 0;
    virtual void setAxisAutoScale( RiuPlotAxis axis, bool enable )        = 0;

    virtual void setAxisMaxMinor( RiuPlotAxis axis, int maxMinor ) = 0;
    virtual void setAxisMaxMajor( RiuPlotAxis axis, int maxMajor ) = 0;

    virtual RiuPlotWidget::AxisScaleType axisScaleType( RiuPlotAxis axis ) const                  = 0;
    virtual void setAxisScaleType( RiuPlotAxis axis, RiuPlotWidget::AxisScaleType axisScaleType ) = 0;

    virtual void setAxisTitleText( RiuPlotAxis axis, const QString& title ) = 0;
    virtual void setAxisTitleEnabled( RiuPlotAxis axis, bool enable )       = 0;

    virtual bool        isMultiAxisSupported() const                         = 0;
    virtual RiuPlotAxis createNextPlotAxis( RiaDefines::PlotAxis axis )      = 0;
    virtual void        pruneAxes( const std::set<RiuPlotAxis>& usedAxes )   = 0;
    virtual void        moveAxis( RiuPlotAxis oldAxis, RiuPlotAxis newAxis ) = 0;

    virtual void   setPlotTitle( const QString& plotTitle ) = 0;
    const QString& plotTitle() const;
    virtual void   setPlotTitleEnabled( bool enabled )       = 0;
    virtual bool   plotTitleEnabled() const                  = 0;
    virtual void   setPlotTitleFontSize( int titleFontSize ) = 0;

    virtual void setLegendFontSize( int fontSize )        = 0;
    virtual void setInternalLegendVisible( bool visible ) = 0;
    virtual void insertLegend( RiuPlotWidget::Legend )    = 0;
    virtual void clearLegend()                            = 0;
    virtual void updateLegend()                           = 0;

    virtual void detachItems( RiuPlotWidget::PlotItemType plotItemType ) = 0;

    virtual std::pair<double, double> axisRange( RiuPlotAxis axis ) const                      = 0;
    virtual void                      setAxisRange( RiuPlotAxis axis, double min, double max ) = 0;

    virtual void setAxisInverted( RiuPlotAxis axis, bool isInverted )                                  = 0;
    virtual void setAxisLabelsAndTicksEnabled( RiuPlotAxis axis, bool enableLabels, bool enableTicks ) = 0;

    virtual void enableGridLines( RiuPlotAxis axis, bool majorGridLines, bool minorGridLines ) = 0;

    virtual void
                 setMajorTicksList( RiuPlotAxis axis, const QList<double>& majorTicks, double minValue, double maxValue ) = 0;
    virtual void setMajorAndMinorTickIntervals( RiuPlotAxis axis,
                                                double      majorTickInterval,
                                                double      minorTickInterval,
                                                double      minValue,
                                                double      maxValue )         = 0;
    virtual void setMajorAndMinorTickIntervalsAndRange( RiuPlotAxis axis,
                                                        double      majorTickInterval,
                                                        double      minorTickInterval,
                                                        double      minTickValue,
                                                        double      maxTickValue,
                                                        double      rangeMin,
                                                        double      rangeMax ) = 0;

    virtual void
        setAutoTickIntervalCounts( RiuPlotAxis axis, int maxMajorTickIntervalCount, int maxMinorTickIntervalCount ) = 0;

    virtual double majorTickInterval( RiuPlotAxis axis ) const = 0;
    virtual double minorTickInterval( RiuPlotAxis axis ) const = 0;

    virtual int axisExtent( RiuPlotAxis axis ) const = 0;

    virtual void ensureAxisIsCreated( RiuPlotAxis axis ) = 0;

    QPoint dragStartPosition() const;

    void         scheduleReplot();
    virtual void replot() = 0;

    void addOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void removeOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void clearOverlayFrames( std::function<bool( RiuDraggableOverlayFrame* )> matcher );

    virtual void updateLayout() = 0;

    virtual void renderTo( QPainter* painter, const QRect& targetRect, double scaling ) = 0;
    virtual void renderTo( QPaintDevice* painter, const QRect& targetRect )             = 0;
    int          overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

    virtual void removeEventFilter();

    virtual void updateAxes() = 0;

    virtual RiuPlotCurve* createPlotCurve( RimPlotCurve* ownerRimCurve, const QString& title ) = 0;

    virtual const QColor& backgroundColor() const = 0;

    virtual QWidget* getParentForOverlay() const = 0;

    virtual std::pair<RiuPlotCurve*, int> findClosestCurve( const QPoint& pos, double& distanceToClick ) const = 0;

    virtual void updateZoomDependentCurveProperties();

signals:
    void curveOrderNeedsUpdate();

protected:
    void updateOverlayFrameLayout();

    bool handleDragDropEvent( QEvent* event );

    static int defaultMinimumWidth();

    caf::PdmPointer<RimPlot>       m_plotDefinition;
    QPoint                         m_clickPosition;
    std::map<RiuPlotAxis, QString> m_axisTitles;
    std::map<RiuPlotAxis, bool>    m_axisTitlesEnabled;
    const int                      m_overlayMargins;
    QString                        m_plotTitle;
    bool                           m_plotTitleEnabled;

    QList<QPointer<RiuDraggableOverlayFrame>> m_overlayFrames;
};
