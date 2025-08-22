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
#include "RiuMultiPlotBook.h"

#include "RiaGuiApplication.h"
#include "RiaPlotDefines.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "RimContextCommandBuilder.h"
#include "RimMultiPlot.h"
#include "RimSummaryMultiPlot.h"
#include "RimWellLogTrack.h"

#include "RiuDragDrop.h"
#include "RiuMainWindow.h"
#include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotObjectPicker.h"
#include "RiuPlotWidget.h"
#include "RiuSummaryMultiPlotPage.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QDebug>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPagedPaintDevice>
#include <QPainter>
#include <QScrollArea>
#include <QScrollBar>
#include <QTimer>

#include <algorithm>
#include <cmath>

class BookFrame : public QFrame
{
public:
    BookFrame( int margins, QWidget* parent = nullptr )
        : QFrame( parent )
        , m_margins( margins )
    {
    }
    QSize calculateSize( int width ) const
    {
        int                      pageWidth = width - 2 * m_margins;
        QList<RiuMultiPlotPage*> pages     = findChildren<RiuMultiPlotPage*>();
        QSize                    fullSize( 0, 0 );
        for ( auto page : pages )
        {
            fullSize.setWidth( pageWidth );
            fullSize.setHeight( fullSize.height() + m_margins + page->heightForWidth( pageWidth ) );
        }
        QSize minSize = minimumSizeHint();
        return QSize( std::max( minSize.width(), fullSize.width() ), std::max( minSize.height(), fullSize.height() ) );
    }

protected:
    QSize minimumSizeHint() const override
    {
        QList<RiuMultiPlotPage*> pages = findChildren<RiuMultiPlotPage*>();
        QSize                    fullSize( 0, 0 );
        for ( auto page : pages )
        {
            fullSize.setWidth( std::max( fullSize.width(), page->minimumSizeHint().width() ) );
            fullSize.setHeight( fullSize.height() + m_margins + page->minimumSizeHint().height() );
        }
        return fullSize;
    }

private:
    int m_margins;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotBook::RiuMultiPlotBook( RimMultiPlot* plotDefinition, QWidget* parent )
    : QWidget( parent )
    , m_plotDefinition( plotDefinition )
    , m_plotTitle( "Multi Plot" )
    , m_titleVisible( true )
    , m_subTitlesVisible( true )
    , m_previewMode( true )
    , m_currentPageIndex( 0 )
    , m_goToPageAfterUpdate( false )
    , m_pageTimerId( -1 )
    , m_pageToGoTo( 0 )
{
    const int spacing = 8;

    setContentsMargins( 0, 0, 0, 0 );
    m_layout = new QVBoxLayout( this );
    m_layout->setContentsMargins( 0, 0, 0, 0 );
    m_scrollArea = new QScrollArea( this );
    m_scrollArea->setFrameStyle( QFrame::NoFrame );
    m_scrollArea->setHorizontalScrollBarPolicy( Qt::ScrollBarAlwaysOff );
    m_scrollArea->setVerticalScrollBarPolicy( Qt::ScrollBarAlwaysOn );
    m_layout->addWidget( m_scrollArea );
    m_book = new BookFrame( spacing );
    m_book->setFrameStyle( QFrame::NoFrame );
    m_scrollArea->setWidget( m_book );
    m_scrollArea->setWidgetResizable( true );
    m_bookLayout = new QVBoxLayout( m_book );
    m_bookLayout->setSpacing( spacing );
    m_scrollArea->setVisible( true );
    m_book->setVisible( true );

    m_scrollArea->viewport()->installEventFilter( this );
    m_scrollArea->verticalScrollBar()->installEventFilter( this );

    setAutoFillBackground( true );

    setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    setFocusPolicy( Qt::StrongFocus );
    setAcceptDrops( true );

    QSize pageSize = m_plotDefinition->pageLayout().fullRectPixels( RiaGuiApplication::applicationResolution() ).size();
    applyPagePreviewBookSize( pageSize.width() );
    setObjectName( QString( "%1" ).arg( reinterpret_cast<uint64_t>( this ) ) );

    applyLook();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotBook::~RiuMultiPlotBook()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuMultiPlotBook::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::addPlot( RiuPlotWidget* plotWidget )
{
    // Push the plot to the back of the list
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::insertPlot( RiuPlotWidget* plotWidget, size_t index )
{
    m_plotWidgets.insert( static_cast<int>( index ), plotWidget );
    m_goToPageAfterUpdate = true;
    m_pageToGoTo          = -2;
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::removePlot( RiuPlotWidget* plotWidget )
{
    removePlotNoUpdate( plotWidget );
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::removePlotNoUpdate( RiuPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    int plotWidgetIdx = m_plotWidgets.indexOf( plotWidget );
    CVF_ASSERT( plotWidgetIdx >= 0 );

    m_plotWidgets.removeAt( plotWidgetIdx );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::removeAllPlots()
{
    deleteAllPages();
    m_plotWidgets.clear();
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle = plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setTitleVisible( bool visible )
{
    m_titleVisible = visible;
    for ( auto page : m_pages )
    {
        page->setTitleVisible( visible );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setSubTitlesVisible( bool visible )
{
    m_subTitlesVisible = visible;
    for ( auto page : m_pages )
    {
        page->setSubTitlesVisible( visible );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::scheduleTitleUpdate()
{
    for ( auto page : m_pages )
    {
        page->scheduleUpdate( RiaDefines::MultiPlotPageUpdateType::TITLE );
    }
    scheduleUpdate( RiaDefines::MultiPlotPageUpdateType::TITLE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setTitleFontSizes( int titleFontSize, int subTitleFontSize )
{
    for ( auto page : m_pages )
    {
        page->setTitleFontSizes( titleFontSize, subTitleFontSize );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setLegendFontSize( int legendFontSize )
{
    for ( auto page : m_pages )
    {
        page->setLegendFontSize( legendFontSize );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setAxisFontSizes( int axisTitleFontSize, int axisValueFontSize )
{
    for ( auto page : m_pages )
    {
        page->setAxisFontSizes( axisTitleFontSize, axisValueFontSize );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotBook::indexOfPlotWidget( RiuPlotWidget* plotWidget )
{
    return m_plotWidgets.indexOf( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotBook::pagePreviewModeEnabled() const
{
    return m_previewMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setPagePreviewModeEnabled( bool previewMode )
{
    m_previewMode = previewMode;

    for ( auto page : m_pages )
    {
        page->setPagePreviewModeEnabled( pagePreviewModeEnabled() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::scheduleUpdate( RiaDefines::MultiPlotPageUpdateType whatToUpdate )
{
    RiaPlotWindowRedrawScheduler::instance()->scheduleMultiPlotBookUpdate( this, whatToUpdate );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::scheduleReplotOfAllPlots()
{
    for ( RiuPlotWidget* plotWidget : visiblePlotWidgets() )
    {
        plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::renderTo( QPaintDevice* paintDevice )
{
    auto scaling = RiaDefines::scalingFactor( paintDevice );

    QPainter painter( paintDevice );

    auto pagedDevice = dynamic_cast<QPagedPaintDevice*>( paintDevice );
    if ( pagedDevice )
    {
        bool firstPage = true;
        for ( RiuMultiPlotPage* page : m_pages )
        {
            if ( !firstPage )
            {
                pagedDevice->newPage();
            }
            page->renderTo( &painter, scaling );
            firstPage = false;
        }
    }
    else
    {
        if ( m_currentPageIndex < m_pages.size() )
        {
            auto page = m_pages[m_currentPageIndex];
            page->renderTo( &painter, scaling );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem( ownerViewWindow() );

    menuBuilder << "RicShowPlotDataFeature";
    menuBuilder << "RicShowContributingWellsFromPlotFeature";

    menuBuilder.appendToMenu( &menu );

    if ( !menu.actions().empty() )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::showEvent( QShowEvent* event )
{
    m_goToPageAfterUpdate = true;
    QWidget::showEvent( event );

    if ( m_previewMode )
    {
        applyPagePreviewBookSize( width() );
    }
    else
    {
        applyBookSize( width(), height() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::hideEvent( QHideEvent* event )
{
    m_pageToGoTo = m_currentPageIndex;
    QWidget::hideEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::resizeEvent( QResizeEvent* event )
{
    if ( m_previewMode )
    {
        applyPagePreviewBookSize( event->size().width() );
    }
    else
    {
        applyBookSize( event->size().width(), event->size().height() );
    }
    QWidget::resizeEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::applyPagePreviewBookSize( int frameWidth )
{
    for ( auto page : m_pages )
    {
        int width          = page->width();
        int heightForWidth = page->heightForWidth( width );
        page->resize( width, heightForWidth );
    }
    m_book->setFixedSize( m_book->calculateSize( frameWidth ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::applyBookSize( int frameWidth, int frameHeight )
{
    int totalHeight = 0;
    for ( auto page : m_pages )
    {
        page->resize( frameWidth, frameHeight );
        totalHeight += frameHeight;
    }
    m_book->setFixedSize( QSize( frameWidth, totalHeight ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RiuMultiPlotBook::rowAndColumnCount( int plotWidgetCount ) const
{
    if ( plotWidgetCount == 0 )
    {
        return std::make_pair( 0, 0 );
    }

    int columnCount = std::max( 1, m_plotDefinition->columnCount() );
    int rowCount    = static_cast<int>( std::ceil( plotWidgetCount / static_cast<double>( columnCount ) ) );
    return std::make_pair( rowCount, columnCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotBook::showYAxis( int row, int column ) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::performUpdate( RiaDefines::MultiPlotPageUpdateType whatToUpdate )
{
    if ( !m_plotDefinition || !m_plotDefinition->isValid() || !m_plotDefinition->showWindow() ) return;

    applyLook();
    if ( RiaDefines::isPlotUpdate( whatToUpdate ) || m_pages.empty() )
    {
        deleteAllPages();
        createPages();
    }
    else if ( RiaDefines::isTitleUpdate( whatToUpdate ) )
    {
        updatePageTitles();
    }

    updateGeometry();

    RimSummaryMultiPlot* multiPlot = dynamic_cast<RimSummaryMultiPlot*>( m_plotDefinition.p() );
    if ( multiPlot ) multiPlot->analyzePlotsAndAdjustAppearanceSettings();

    // use a timer to trigger a viewer page change, if needed
    if ( m_goToPageAfterUpdate )
    {
        m_pageTimerId         = startTimer( 50 );
        m_goToPageAfterUpdate = false;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::updatePageTitles()
{
    if ( m_pages.isEmpty() ) return;

    if ( m_pages.size() > 1 )
    {
        for ( int i = 0; i < m_pages.size(); ++i )
        {
            int pageNumber = i + 1;
            m_pages[i]->setPlotTitle( QString( "%1 %2/%3" ).arg( m_plotTitle ).arg( pageNumber ).arg( m_pages.size() ) );
        }
    }
    else
    {
        m_pages[0]->setPlotTitle( QString( "%1" ).arg( m_plotTitle ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::forcePerformUpdate()
{
    performUpdate( RiaDefines::MultiPlotPageUpdateType::ALL );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::timerEvent( QTimerEvent* event )
{
    if ( event->timerId() == m_pageTimerId )
    {
        killTimer( m_pageTimerId );
        changeCurrentPage( m_pageToGoTo );
        RiaGuiApplication::instance()->mainPlotWindow()->raise();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuPlotWidget>> RiuMultiPlotBook::visiblePlotWidgets() const
{
    QList<QPointer<RiuPlotWidget>> plotWidgets;
    for ( QPointer<RiuPlotWidget> plotWidget : m_plotWidgets )
    {
        CAF_ASSERT( plotWidget );
        if ( plotWidget->isChecked() )
        {
            plotWidgets.push_back( plotWidget );
        }
    }
    return plotWidgets;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::deleteAllPages()
{
    for ( RiuMultiPlotPage* page : m_pages )
    {
        m_bookLayout->removeWidget( page );
        page->removeAllPlots();
        delete page;
    }
    m_pages.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::createPages()
{
    CAF_ASSERT( m_plotDefinition );

    QList<QPointer<RiuPlotWidget>> plotWidgets = visiblePlotWidgets();
    auto [rowCount, columnCount]               = rowAndColumnCount( plotWidgets.size() );

    int rowsPerPage = m_plotDefinition->rowsPerPage();
    int row         = 0;
    int column      = 0;

    // Make sure we always add a page. For empty multi-plots we'll have an empty page with a drop target.
    RiuMultiPlotPage* page = createPage();

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int expectedColSpan = static_cast<int>( plotWidgets[visibleIndex]->colSpan() );
        int colSpan         = std::min( expectedColSpan, columnCount );

        std::tie( row, column ) = page->findAvailableRowAndColumn( row, column, colSpan, columnCount );
        if ( row >= rowsPerPage )
        {
            page   = createPage();
            row    = 0;
            column = 0;
        }
        CAF_ASSERT( plotWidgets[visibleIndex] );
        page->addPlot( plotWidgets[visibleIndex] );
        page->performUpdate( RiaDefines::MultiPlotPageUpdateType::ALL );
    }

    // Set page numbers in title when there's more than one page
    updatePageTitles();

    adjustBookFrame();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QList<QPointer<RiuMultiPlotPage>>& RiuMultiPlotBook::pages() const
{
    return m_pages;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotPage* RiuMultiPlotBook::createPage()
{
    RiuMultiPlotPage* page = new RiuMultiPlotPage( m_plotDefinition, this );

    applyPageSettings( page );

    m_pages.push_back( page );
    m_bookLayout->addWidget( page );

    page->setVisible( true );
    return page;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::applyPageSettings( RiuMultiPlotPage* page )
{
    if ( !page ) return;

    page->setPlotTitle( m_plotTitle );
    page->setTitleFontSizes( m_plotDefinition->titleFontSize(), m_plotDefinition->subTitleFontSize() );
    page->setLegendFontSize( m_plotDefinition->legendFontSize() );
    page->setAxisFontSizes( m_plotDefinition->axisTitleFontSize(), m_plotDefinition->axisValueFontSize() );
    page->setTitleVisible( m_titleVisible );
    page->setSubTitlesVisible( m_subTitlesVisible );
    page->setPagePreviewModeEnabled( m_previewMode );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::applyLook()
{
    if ( m_previewMode )
    {
        setBackgroundRole( QPalette::Dark );
        m_book->setBackgroundRole( QPalette::Dark );
    }
    else
    {
        QPalette newPalette( palette() );
        newPalette.setColor( QPalette::Window, Qt::white );
        setPalette( newPalette );

        setBackgroundRole( QPalette::Window );
        m_book->setBackgroundRole( QPalette::Window );
        m_book->setPalette( newPalette );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::changeCurrentPage( int pageNumber )
{
    if ( pageNumber < -1 ) pageNumber = m_pages.size() - 1;
    m_currentPageIndex = pageNumber;
    if ( m_currentPageIndex >= m_pages.size() ) m_currentPageIndex = m_pages.size() - 1;
    if ( m_currentPageIndex < 0 ) m_currentPageIndex = 0;
    if ( !m_pages.isEmpty() ) m_scrollArea->ensureWidgetVisible( m_pages[m_currentPageIndex] );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::scrollToPlot( RiuPlotWidget* plotWidget )
{
    int pageNum = 0;

    for ( auto& page : m_pages )
    {
        if ( page->indexOfPlotWidget( plotWidget ) >= 0 )
        {
            changeCurrentPage( pageNum );
            return;
        }
        pageNum++;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::goToNextPage()
{
    changeCurrentPage( m_currentPageIndex + 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::goToPrevPage()
{
    changeCurrentPage( m_currentPageIndex - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::goToLastPage()
{
    changeCurrentPage( m_pages.size() - 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::keepCurrentPageAfterUpdate()
{
    m_goToPageAfterUpdate = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::dragEnterEvent( QDragEnterEvent* event )
{
    event->acceptProposedAction();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::dropEvent( QDropEvent* event )
{
    std::vector<caf::PdmObjectHandle*> objects;

    if ( RiuDragDrop::handleGenericDropEvent( event, objects ) )
    {
        RimSummaryMultiPlot* multiPlot = dynamic_cast<RimSummaryMultiPlot*>( m_plotDefinition.p() );
        if ( multiPlot )
        {
            multiPlot->handleDroppedObjects( objects );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotBook::eventFilter( QObject* obj, QEvent* event )
{
    if ( event->type() == QEvent::Wheel )
    {
        event->accept();
        return true;
    }
    return QWidget::eventFilter( obj, event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::adjustBookFrame()
{
    m_book->adjustSize();
}
