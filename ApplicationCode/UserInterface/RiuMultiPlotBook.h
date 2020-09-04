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
#include "RiuMultiPlotPage.h"

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
class RimMultiPlot;
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
// RiuMultiPlotWindow
//
//==================================================================================================
class RiuMultiPlotBook : public QWidget, public RiuInterfaceToViewWindow
{
    Q_OBJECT

public:
    using ColumnCount = RiuMultiPlotPage::ColumnCount;

public:
    RiuMultiPlotBook( RimMultiPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuMultiPlotBook() override;

    RimViewWindow* ownerViewWindow() const override;

    void addPlot( RiuQwtPlotWidget* plotWidget );
    void insertPlot( RiuQwtPlotWidget* plotWidget, size_t index );
    void removePlot( RiuQwtPlotWidget* plotWidget );

    void setPlotTitle( const QString& plotTitle );

    void setTitleVisible( bool visible );
    void setSubTitlesVisible( bool visible );

    void setTitleFontSizes( int titleFontSize, int subTitleFontSize );
    void setLegendFontSize( int legendFontSize );
    void setAxisFontSizes( int axisTitleFontSize, int axisValueFontSize );

    int indexOfPlotWidget( RiuQwtPlotWidget* plotWidget );

    bool pagePreviewModeEnabled() const;
    void setPagePreviewModeEnabled( bool previewMode );

    void scheduleUpdate();
    void scheduleReplotOfAllPlots();

    void renderTo( QPaintDevice* painter );

protected:
    void contextMenuEvent( QContextMenuEvent* ) override;

    void showEvent( QShowEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;

    void applyPagePreviewBookSize( int frameWidth );
    void applyBookSize( int frameWidth, int frameHeight );

    std::pair<int, int> rowAndColumnCount( int plotWidgetCount ) const;

    virtual bool showYAxis( int row, int column ) const;

    QList<QPointer<RiuQwtPlotWidget>> visiblePlotWidgets() const;

private:
    void                                     deleteAllPages();
    void                                     createPages();
    const QList<QPointer<RiuMultiPlotPage>>& pages() const;
    RiuMultiPlotPage*                        createPage();
    void                                     applyLook();
private slots:
    virtual void performUpdate();

protected:
    friend class RiaPlotWindowRedrawScheduler;

    QPointer<QVBoxLayout> m_layout;
    QPointer<QScrollArea> m_scrollArea;
    QPointer<BookFrame>   m_book;
    QPointer<QVBoxLayout> m_bookLayout;

    QList<QPointer<RiuMultiPlotPage>> m_pages;
    QList<QPointer<RiuQwtPlotWidget>> m_plotWidgets;
    caf::PdmPointer<RimMultiPlot>     m_plotDefinition;
    QString                           m_plotTitle;
    bool                              m_titleVisible;
    bool                              m_subTitlesVisible;
    bool                              m_previewMode;
};
