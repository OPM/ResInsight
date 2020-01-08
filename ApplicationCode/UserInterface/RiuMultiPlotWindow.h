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
class RiuMultiPlotPage;
class RiuQwtPlotWidget;

class BookFrame;
class QFocusEvent;
class QLabel;
class QPaintDevice;
class QScrollArea;
class QScrollBar;
class QwtPlot;

//==================================================================================================
//
// RiuMultiPlotWidget
//
//==================================================================================================
class RiuMultiPlotWindow : public RiuMultiPlotInterface, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    RiuMultiPlotWindow( RimMultiPlotWindow* plotDefinition, QWidget* parent = nullptr );
    ~RiuMultiPlotWindow() override;

    RimMultiPlotWindow* ownerPlotDefinition();
    RimViewWindow*      ownerViewWindow() const override;

    void addPlot( RiuQwtPlotWidget* plotWidget ) override;
    void insertPlot( RiuQwtPlotWidget* plotWidget, size_t index ) override;
    void removePlot( RiuQwtPlotWidget* plotWidget ) override;

    void setPlotTitle( const QString& plotTitle ) override;

    void setTitleVisible( bool visible ) override;
    void setSelectionsVisible( bool visible );

    void setFontSize( int fontSize );
    int  fontSize() const;

    int indexOfPlotWidget( RiuQwtPlotWidget* plotWidget );

    bool previewModeEnabled() const override;
    void setPreviewModeEnabled( bool previewMode ) override;

    void scheduleUpdate();
    void scheduleReplotOfAllPlots();
    void updateVerticalScrollBar( double visibleMin, double visibleMax, double totalMin, double totalMax ) override {}

    void renderTo( QPaintDevice* painter ) override;

protected:
    void contextMenuEvent( QContextMenuEvent* ) override;

    void showEvent( QShowEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;

    void setBookSize( int frameWidth );

    std::pair<int, int> rowAndColumnCount( int plotWidgetCount ) const;

    virtual bool showYAxis( int row, int column ) const;

    QList<QPointer<RiuQwtPlotWidget>> visiblePlotWidgets() const;

private:
    void                                     deleteAllPages();
    void                                     createPages();
    const QList<QPointer<RiuMultiPlotPage>>& pages() const;

private slots:
    virtual void performUpdate();

protected:
    friend class RiaPlotWindowRedrawScheduler;

    QPointer<QVBoxLayout> m_layout;
    QPointer<QScrollArea> m_scrollArea;
    QPointer<BookFrame>   m_book;
    QPointer<QVBoxLayout> m_bookLayout;

    QList<QPointer<RiuMultiPlotPage>>   m_pages;
    QList<QPointer<RiuQwtPlotWidget>>   m_plotWidgets;
    caf::PdmPointer<RimMultiPlotWindow> m_plotDefinition;
    QString                             m_plotTitle;
    bool                                m_titleVisible;
    bool                                m_previewMode;
};
