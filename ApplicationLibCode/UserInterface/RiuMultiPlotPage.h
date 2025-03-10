/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RiuDraggableOverlayFrame.h"
#include "RiuInterfaceToViewWindow.h"

#include "RiaDefines.h"

#include "cafPdmPointer.h"

#include "qwt_axis_id.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QList>
#include <QPointer>
#include <QWidget>

#include <functional>
#include <map>

class RiaPlotWindowRedrawScheduler;
class RimPlotWindow;
class RiuQwtPlotLegend;
class RiuPlotWidget;

class QFocusEvent;
class QLabel;
class QPaintDevice;
class QScrollBar;
class QwtLegend;
class QwtLegendData;
class QwtPlot;

//==================================================================================================
//
// RiuMultiPlotPage
//
//==================================================================================================
class RiuMultiPlotPage : public QWidget, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    RiuMultiPlotPage( RimPlotWindow* plotDefinition, QWidget* parent = nullptr );
    ~RiuMultiPlotPage() override;

    RimViewWindow* ownerViewWindow() const override;
    RimPlotWindow* ownerPlotDefinition();

    void addPlot( RiuPlotWidget* plotWidget );
    void insertPlot( RiuPlotWidget* plotWidget, size_t index );
    void removePlot( RiuPlotWidget* plotWidget );
    void removeAllPlots();
    int  indexOfPlotWidget( RiuPlotWidget* plotWidget );

    void setPlotTitle( const QString& plotTitle );
    void setTitleVisible( bool visible );
    void setTitleFontSizes( int titleFontSize, int subTitleFontSize );
    void setLegendFontSize( int legendFontSize );
    void setAxisFontSizes( int axisTitleFontSize, int axisValueFontSize );
    void setSubTitlesVisible( bool visible );

    bool previewModeEnabled() const;
    void setPagePreviewModeEnabled( bool previewMode );

    void setAutoAlignAxes( bool autoAlignAxes );

    void         scheduleUpdate( RiaDefines::MultiPlotPageUpdateType whatToUpdate = RiaDefines::MultiPlotPageUpdateType::ALL );
    void         scheduleReplotOfAllPlots();
    virtual void updateVerticalScrollBar( double visibleMin, double visibleMax, double totalMin, double totalMax ) {}
    void         updateSubTitles();

    virtual void renderTo( QPaintDevice* paintDevice );
    void         renderTo( QPainter* painter, double scalingFactor );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    int   heightForWidth( int width ) const override;

    std::pair<int, int> findAvailableRowAndColumn( int startRow, int startColumn, int columnSpan, int columnCount ) const;

public slots:
    virtual void performUpdate( RiaDefines::MultiPlotPageUpdateType whatToUpdate );

protected:
    void    contextMenuEvent( QContextMenuEvent* ) override;
    QLabel* createTitleLabel() const;

    void showEvent( QShowEvent* event ) override;
    bool hasHeightForWidth() const override;
    void updateMarginsFromPageLayout();

    void updateLegendColumns( RiuQwtPlotLegend* legend );
    void updateLegendFont( RiuQwtPlotLegend* legend );

    std::pair<int, int> rowAndColumnCount( int plotWidgetCount ) const;

    void alignAxes();
    void alignAxis( QwtAxisId axis, int row, std::function<bool( int, int, int )> positionMatcher );

    virtual bool showYAxis( int row, int column ) const;

    virtual void reinsertPlotWidgets();
    virtual void refreshLegends();
    void         updatePlotLayouts();

    void addLegendWidget( RiuPlotWidget* plotWidget, RiuQwtPlotLegend* legend, RiuDraggableOverlayFrame* legendFrame, int row, int column, int colSpan );

    void reinsertPlotWidget( RiuPlotWidget*            plotWidget,
                             RiuQwtPlotLegend*         legend,
                             RiuDraggableOverlayFrame* legendFrame,
                             QLabel*                   subTitle,
                             int                       row,
                             int                       column,
                             int                       rowSpan,
                             int                       colSpan );

    void updateSubTitleVisibility( QLabel* subTitle );

    void setDefaultAxisProperties( RiuPlotWidget* plotWidget, int row, int column );

    void adjustHeadingSpacing( RiuPlotWidget* plotWidget );

    void setRowAndColumnStretches( int row, int column, int rowSpan, int colSpan );

    void updateLegendVisibility( RiuPlotWidget* plotWidget, RiuQwtPlotLegend* legend, RiuDraggableOverlayFrame* legendFrame );

    void updateTitleFont();

    int alignCanvasTops();

    void clearGridLayout();

    QList<QPointer<RiuPlotWidget>>            visiblePlotWidgets() const;
    QList<QPointer<RiuQwtPlotLegend>>         legendsForVisiblePlots() const;
    QList<QPointer<QLabel>>                   subTitlesForVisiblePlots() const;
    QList<QPointer<RiuDraggableOverlayFrame>> legendFramesForVisiblePlots() const;

    void applyLook();

private slots:
    void onLegendUpdated();

protected:
    QPointer<QVBoxLayout>                     m_layout;
    QPointer<QHBoxLayout>                     m_plotLayout;
    QPointer<QFrame>                          m_plotWidgetFrame;
    QPointer<QGridLayout>                     m_gridLayout;
    QPointer<QLabel>                          m_plotTitle;
    QList<QPointer<QLabel>>                   m_subTitles;
    QList<QPointer<RiuQwtPlotLegend>>         m_legends;
    QList<QPointer<RiuDraggableOverlayFrame>> m_legendFrames;
    QList<QPointer<RiuPlotWidget>>            m_plotWidgets;
    std::map<int, std::pair<int, int>>        m_visibleIndexToPositionMapping;
    caf::PdmPointer<RimPlotWindow>            m_plotDefinition;

    int m_titleFontSize;
    int m_subTitleFontSize;
    int m_legendFontSize;
    int m_axisTitleFontSize;
    int m_axisValueFontSize;

    bool m_previewMode;
    bool m_showSubTitles;
    bool m_autoAlignAxes;

    std::map<RiuQwtPlotLegend*, int> m_childCountForAdjustSizeOperation;

private:
    friend class RiaPlotWindowRedrawScheduler;
};
