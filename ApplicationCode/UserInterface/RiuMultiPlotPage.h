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

#include "RiuMultiPlotInterface.h"

#include "cafUiStyleSheet.h"

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
class RimMultiPlotWindow;
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
class RiuMultiPlotPage : public RiuMultiPlotInterface, public caf::SelectionChangedReceiver
{
    Q_OBJECT

public:
    RiuMultiPlotPage( RimMultiPlotWindow* plotDefinition, QWidget* parent = nullptr );
    ~RiuMultiPlotPage() override;

    RimMultiPlotWindow* ownerPlotDefinition();

    void addPlot( RiuQwtPlotWidget* plotWidget ) override;
    void insertPlot( RiuQwtPlotWidget* plotWidget, size_t index ) override;
    void removePlot( RiuQwtPlotWidget* plotWidget ) override;
    void removeAllPlots();
    int  indexOfPlotWidget( RiuQwtPlotWidget* plotWidget ) override;

    void setSelectionsVisible( bool visible );

    void setPlotTitle( const QString& plotTitle ) override;
    void setTitleVisible( bool visible ) override;
    void setFontSize( int fontSize ) override;
    int  fontSize() const override;

    void scheduleUpdate() override;
    void scheduleReplotOfAllPlots() override;
    void updateVerticalScrollBar( double visibleMin, double visibleMax, double totalMin, double totalMax ) override {}

    void renderTo( QPaintDevice* paintDevice ) override;
    void renderTo( QPainter* painter );

    QSize sizeHint() const override;
    QSize minimumSizeHint() const override;
    int   heightForWidth( int width ) const override;

protected:
    void    contextMenuEvent( QContextMenuEvent* ) override;
    QLabel* createTitleLabel() const;

    void showEvent( QShowEvent* event ) override;
    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dragMoveEvent( QDragMoveEvent* event ) override;
    void dragLeaveEvent( QDragLeaveEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;
    bool hasHeightForWidth() const override;
    void updateMarginsFromPageLayout();

    virtual bool willAcceptDroppedPlot( const RiuQwtPlotWidget* plotWidget ) const;

    std::pair<int, int> rowAndColumnCount( int plotWidgetCount ) const;

    virtual void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;

    void setWidgetState( const QString& widgetState );

    virtual bool showYAxis( int row, int column ) const;

    void reinsertPlotWidgets();
    int  alignCanvasTops();

    void              clearGridLayout();
    caf::UiStyleSheet createDropTargetStyleSheet();

    QList<QPointer<RiuQwtPlotWidget>> visiblePlotWidgets() const;
    QList<QPointer<RiuQwtPlotLegend>> legendsForVisiblePlots() const;
    QList<QPointer<QLabel>>           subTitlesForVisiblePlots() const;

    std::pair<int, int> findAvailableRowAndColumn( int startRow, int startColumn, int columnSpan, int columnCount ) const;

private slots:
    virtual void performUpdate();
    void         onLegendUpdated();

protected:
    friend class RiuMultiPlotWindow;

    QPointer<QVBoxLayout>               m_layout;
    QPointer<QHBoxLayout>               m_plotLayout;
    QPointer<QFrame>                    m_plotWidgetFrame;
    QPointer<QGridLayout>               m_gridLayout;
    QPointer<QLabel>                    m_plotTitle;
    QList<QPointer<QLabel>>             m_subTitles;
    QList<QPointer<RiuQwtPlotLegend>>   m_legends;
    QList<QPointer<RiuQwtPlotWidget>>   m_plotWidgets;
    caf::PdmPointer<RimMultiPlotWindow> m_plotDefinition;
    QPointer<QLabel>                    m_dropTargetPlaceHolder;

    caf::UiStyleSheet m_dropTargetStyleSheet;

private:
    friend class RiaPlotWindowRedrawScheduler;
};
