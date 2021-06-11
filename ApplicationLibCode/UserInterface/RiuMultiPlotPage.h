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

#include "RiuInterfaceToViewWindow.h"

#include "cafPdmPointer.h"
#include "cafSelectionChangedReceiver.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QList>
#include <QPointer>
#include <QWidget>

#include <map>

class RiaPlotWindowRedrawScheduler;
class RimPlotWindow;
class RiuQwtPlotLegend;
class RiuQwtPlotWidget;

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
class RiuMultiPlotPage : public QWidget, public caf::SelectionChangedReceiver, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    enum class ColumnCount
    {
        COLUMNS_1         = 1,
        COLUMNS_2         = 2,
        COLUMNS_3         = 3,
        COLUMNS_4         = 4,
        COLUMNS_UNLIMITED = 1000,
    };

public:
    RiuMultiPlotPage( RimPlotWindow* plotDefinition, QWidget* parent = nullptr );
    ~RiuMultiPlotPage() override;

    RimViewWindow* ownerViewWindow() const override;
    RimPlotWindow* ownerPlotDefinition();

    void addPlot( RiuQwtPlotWidget* plotWidget );
    void insertPlot( RiuQwtPlotWidget* plotWidget, size_t index );
    void removePlot( RiuQwtPlotWidget* plotWidget );
    void removeAllPlots();
    int  indexOfPlotWidget( RiuQwtPlotWidget* plotWidget );

    void setPlotTitle( const QString& plotTitle );
    void setTitleVisible( bool visible );
    void setTitleFontSizes( int titleFontSize, int subTitleFontSize );
    void setLegendFontSize( int legendFontSize );
    void setAxisFontSizes( int axisTitleFontSize, int axisValueFontSize );
    void setSubTitlesVisible( bool visible );

    bool previewModeEnabled() const;
    void setPagePreviewModeEnabled( bool previewMode );

    void         scheduleUpdate();
    void         scheduleReplotOfAllPlots();
    virtual void updateVerticalScrollBar( double visibleMin, double visibleMax, double totalMin, double totalMax ) {}
    void         updateSubTitles();

    virtual void renderTo( QPaintDevice* paintDevice );
    void         renderTo( QPainter* painter, double scalingFactor );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    int   heightForWidth( int width ) const override;

protected:
    void    contextMenuEvent( QContextMenuEvent* ) override;
    QLabel* createTitleLabel() const;

    void showEvent( QShowEvent* event ) override;
    bool hasHeightForWidth() const override;
    void updateMarginsFromPageLayout();

    std::pair<int, int> rowAndColumnCount( int plotWidgetCount ) const;

    void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;

    virtual bool showYAxis( int row, int column ) const;

    void reinsertPlotWidgets();
    int  alignCanvasTops();

    void clearGridLayout();

    QList<QPointer<RiuQwtPlotWidget>> visiblePlotWidgets() const;
    QList<QPointer<RiuQwtPlotLegend>> legendsForVisiblePlots() const;
    QList<QPointer<QLabel>>           subTitlesForVisiblePlots() const;

    std::pair<int, int> findAvailableRowAndColumn( int startRow, int startColumn, int columnSpan, int columnCount ) const;

    void applyLook();
private slots:
    virtual void performUpdate();
    void         onLegendUpdated();

protected:
    friend class RiuMultiPlotBook;

    QPointer<QVBoxLayout>             m_layout;
    QPointer<QHBoxLayout>             m_plotLayout;
    QPointer<QFrame>                  m_plotWidgetFrame;
    QPointer<QGridLayout>             m_gridLayout;
    QPointer<QLabel>                  m_plotTitle;
    QList<QPointer<QLabel>>           m_subTitles;
    QList<QPointer<RiuQwtPlotLegend>> m_legends;
    QList<QPointer<RiuQwtPlotWidget>> m_plotWidgets;
    caf::PdmPointer<RimPlotWindow>    m_plotDefinition;

    int m_titleFontPixelSize;
    int m_subTitleFontPixelSize;
    int m_legendFontPixelSize;
    int m_axisTitleFontSize;
    int m_axisValueFontSize;

    bool m_previewMode;
    bool m_showSubTitles;

private:
    friend class RiaPlotWindowRedrawScheduler;
};
