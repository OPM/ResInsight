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
class RiuPlotWidget;

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
    RiuMultiPlotBook( RimMultiPlot* plotDefinition, QWidget* parent = nullptr );
    ~RiuMultiPlotBook() override;

    RimViewWindow* ownerViewWindow() const override;

    void addPlot( RiuPlotWidget* plotWidget );
    void insertPlot( RiuPlotWidget* plotWidget, size_t index );
    void removePlot( RiuPlotWidget* plotWidget );
    void removePlotNoUpdate( RiuPlotWidget* plotWidget );
    void removeAllPlots();

    void setPlotTitle( const QString& plotTitle );

    void setTitleVisible( bool visible );
    void setSubTitlesVisible( bool visible );
    void scheduleTitleUpdate();

    void setTitleFontSizes( int titleFontSize, int subTitleFontSize );
    void setLegendFontSize( int legendFontSize );
    void setAxisFontSizes( int axisTitleFontSize, int axisValueFontSize );

    int indexOfPlotWidget( RiuPlotWidget* plotWidget );

    bool pagePreviewModeEnabled() const;
    void setPagePreviewModeEnabled( bool previewMode );

    void scheduleUpdate( RiaDefines::MultiPlotPageUpdateType whatToUpdate = RiaDefines::MultiPlotPageUpdateType::ALL );
    void scheduleReplotOfAllPlots();

    void renderTo( QPaintDevice* painter );

    void scrollToPlot( RiuPlotWidget* plotWidget );

    void goToNextPage();
    void goToPrevPage();
    void goToLastPage();

    void keepCurrentPageAfterUpdate();

    // https://github.com/OPM/ResInsight/issues/10349
    // This function is used to force an update of the plot book. It is intended to be used from RimMultiPlot::onLoadDataAndUpdate()
    // The code used to be called from RiuMultiPlotBook::showEvent(), but this caused a crash when a dock widget was hidden and shown again.
    void forcePerformUpdate();

protected:
    void contextMenuEvent( QContextMenuEvent* ) override;

    void showEvent( QShowEvent* event ) override;
    void hideEvent( QHideEvent* event ) override;
    void resizeEvent( QResizeEvent* event ) override;

    void applyPagePreviewBookSize( int frameWidth );
    void applyBookSize( int frameWidth, int frameHeight );

    std::pair<int, int> rowAndColumnCount( int plotWidgetCount ) const;

    virtual bool showYAxis( int row, int column ) const;

    QList<QPointer<RiuPlotWidget>> visiblePlotWidgets() const;

    void dragEnterEvent( QDragEnterEvent* event ) override;
    void dropEvent( QDropEvent* event ) override;

    void timerEvent( QTimerEvent* event ) override;

    bool eventFilter( QObject* obj, QEvent* ev ) override;

    virtual void createPages();

    void adjustBookFrame();
    void applyPageSettings( RiuMultiPlotPage* page );

    const QList<QPointer<RiuMultiPlotPage>>& pages() const;

    void updatePageTitles();

private:
    RiuMultiPlotPage* createPage();
    void              deleteAllPages();
    void              applyLook();

    void changeCurrentPage( int pageNumber );

private slots:
    virtual void performUpdate( RiaDefines::MultiPlotPageUpdateType updateType );

protected:
    friend class RiaPlotWindowRedrawScheduler;

    QPointer<QVBoxLayout> m_layout;
    QPointer<QScrollArea> m_scrollArea;
    QPointer<BookFrame>   m_book;
    QPointer<QVBoxLayout> m_bookLayout;

    QList<QPointer<RiuMultiPlotPage>> m_pages;
    QList<QPointer<RiuPlotWidget>>    m_plotWidgets;
    caf::PdmPointer<RimMultiPlot>     m_plotDefinition;
    QString                           m_plotTitle;
    bool                              m_titleVisible;
    bool                              m_subTitlesVisible;
    bool                              m_previewMode;
    int                               m_currentPageIndex;

    bool m_goToPageAfterUpdate;
    int  m_pageToGoTo;
    int  m_pageTimerId;
};
