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

#include "cafPdmObject.h"
#include "cafPdmPointer.h"

#include <QPointer>
#include <QWidget>

class RiaPlotWindowRedrawScheduler;
class RimPlot;
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

    virtual int  axisTitleFontSize( RiaDefines::PlotAxis axis ) const             = 0;
    virtual int  axisValueFontSize( RiaDefines::PlotAxis axis ) const             = 0;
    virtual void setAxisFontsAndAlignment( RiaDefines::PlotAxis,
                                           int  titleFontSize,
                                           int  valueFontSize,
                                           bool titleBold = false,
                                           int  alignment = (int)Qt::AlignCenter ) = 0;
    virtual void setAxesFontsAndAlignment( int  titleFontSize,
                                           int  valueFontSize,
                                           bool titleBold = false,
                                           int  alignment = (int)Qt::AlignCenter ) = 0;

    virtual void enableAxis( RiaDefines::PlotAxis axis, bool isEnabled ) = 0;
    virtual bool axisEnabled( RiaDefines::PlotAxis axis ) const          = 0;

    virtual void setAxisScale( RiaDefines::PlotAxis axis, double min, double max ) = 0;
    virtual void setAxisAutoScale( RiaDefines::PlotAxis axis, bool enable )        = 0;

    virtual void setAxisMaxMinor( RiaDefines::PlotAxis axis, int maxMinor ) = 0;
    virtual void setAxisMaxMajor( RiaDefines::PlotAxis axis, int maxMajor ) = 0;

    virtual RiuPlotWidget::AxisScaleType axisScaleType( RiaDefines::PlotAxis axis ) const                  = 0;
    virtual void setAxisScaleType( RiaDefines::PlotAxis axis, RiuPlotWidget::AxisScaleType axisScaleType ) = 0;

    virtual void setAxisTitleText( RiaDefines::PlotAxis axis, const QString& title ) = 0;
    virtual void setAxisTitleEnabled( RiaDefines::PlotAxis axis, bool enable )       = 0;

    virtual void   setPlotTitle( const QString& plotTitle ) = 0;
    const QString& plotTitle() const;
    void           setPlotTitleEnabled( bool enabled );
    bool           plotTitleEnabled() const;
    virtual void   setPlotTitleFontSize( int titleFontSize ) = 0;

    virtual void setLegendFontSize( int fontSize )        = 0;
    virtual void setInternalLegendVisible( bool visible ) = 0;
    virtual void insertLegend( RiuPlotWidget::Legend )    = 0;
    virtual void clearLegend()                            = 0;
    virtual void updateLegend()                           = 0;

    virtual void detachItems( RiuPlotWidget::PlotItemType plotItemType ) = 0;

    virtual std::pair<double, double> axisRange( RiaDefines::PlotAxis axis ) const                      = 0;
    virtual void                      setAxisRange( RiaDefines::PlotAxis axis, double min, double max ) = 0;

    virtual void setAxisInverted( RiaDefines::PlotAxis axis, bool isInverted ) = 0;
    void         setAxisLabelsAndTicksEnabled( RiaDefines::PlotAxis axis, bool enableLabels, bool enableTicks );

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

    void         scheduleReplot();
    virtual void replot() = 0;

    void addOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void removeOverlayFrame( RiuDraggableOverlayFrame* overlayWidget );
    void updateLayout();

    virtual void renderTo( QPainter* painter, const QRect& targetRect, double scaling ) = 0;
    virtual void renderTo( QPaintDevice* painter, const QRect& targetRect )             = 0;
    int          overlayMargins() const;

    RimViewWindow* ownerViewWindow() const override;

    // QwtPlot* qwtPlot() const;

    virtual void removeEventFilter() = 0;

    virtual void updateAxes() = 0;

    virtual RiuPlotCurve* createPlotCurve( const QString& title, const QColor& color ) = 0;

protected:
    static int defaultMinimumWidth();

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
};
