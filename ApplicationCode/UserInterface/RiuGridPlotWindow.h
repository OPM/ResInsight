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

#include "RiuInterfaceToViewWindow.h"
#include "RiuWidgetStyleSheet.h"

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
class RimGridPlotWindow;
class RiuQwtPlotLegend;
class RiuQwtPlotWidget;

class QFocusEvent;
class QLabel;
class QScrollBar;

//==================================================================================================
//
// RiuGridPlotWindow
//
//==================================================================================================
class RiuGridPlotWindow : public QWidget, public RiuInterfaceToViewWindow, public caf::SelectionChangedReceiver
{
    Q_OBJECT

public:
    RiuGridPlotWindow( RimGridPlotWindow* plotDefinition, QWidget* parent = nullptr );

    ~RiuGridPlotWindow() override;

    RimGridPlotWindow* ownerPlotDefinition();
    RimViewWindow*     ownerViewWindow() const override;

    void addPlot( RiuQwtPlotWidget* plotWidget );
    void insertPlot( RiuQwtPlotWidget* plotWidget, size_t index );
    void removePlot( RiuQwtPlotWidget* plotWidget );

    void setPlotTitle( const QString& plotTitle );

    void setTitleVisible( bool visible );
    void setScrollbarVisible( bool visible );
    void setSelectionsVisible( bool visible );

    void setFontSize( int fontSize );
    int  fontSize() const;

    int indexOfPlotWidget( RiuQwtPlotWidget* plotWidget );

    void         scheduleUpdate();
    void         scheduleReplotOfAllPlots();
    virtual void updateVerticalScrollBar( double visibleMin, double visibleMax, double totalMin, double totalMax ) {}

protected:
    void    contextMenuEvent( QContextMenuEvent* ) override;
    void    keyPressEvent( QKeyEvent* keyEvent ) override;
    QLabel* createTitleLabel() const;

    void resizeEvent( QResizeEvent* event ) override;
    void showEvent( QShowEvent* event ) override;
    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dragMoveEvent( QDragMoveEvent* event ) override;
    void dragLeaveEvent( QDragLeaveEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;

    std::pair<int, int> rowAndColumnCount( int plotWidgetCount ) const;

    virtual void onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels ) override;

    void setWidgetState( RiuWidgetStyleSheet::StateTag widgetState );
private slots:
    void performUpdate();

private:
    void                alignCanvasTopsAndScrollbar();
    void                reinsertPlotWidgetsAndScrollbar();
    void                clearGridLayout();
    RiuWidgetStyleSheet createDropTargetStyleSheet();

    QList<QPointer<RiuQwtPlotWidget>> visiblePlotWidgets() const;
    QList<QPointer<RiuQwtPlotLegend>> visibleLegends() const;

protected:
    QPointer<QVBoxLayout>              m_layout;
    QPointer<QHBoxLayout>              m_plotLayout;
    QPointer<QFrame>                   m_plotWidgetFrame;
    QPointer<QGridLayout>              m_gridLayout;
    QPointer<QLabel>                   m_plotTitle;
    QPointer<QVBoxLayout>              m_scrollBarLayout;
    QScrollBar*                        m_scrollBar;
    QList<QPointer<RiuQwtPlotLegend>>  m_legends;
    QList<int>                         m_legendColumns;
    QList<QPointer<RiuQwtPlotWidget>>  m_plotWidgets;
    caf::PdmPointer<RimGridPlotWindow> m_plotDefinition;
    QPointer<QLabel>                   m_dropTargetPlaceHolder;

    RiuWidgetStyleSheet m_dropTargetStyleSheet;

private:
    friend class RiaPlotWindowRedrawScheduler;
};
