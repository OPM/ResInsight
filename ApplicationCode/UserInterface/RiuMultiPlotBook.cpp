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
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "RimContextCommandBuilder.h"
#include "RimMultiPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainWindow.h"
#include "RiuMultiPlotPage.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotObjectPicker.h"
#include "RiuQwtPlotLegend.h"
#include "RiuQwtPlotWidget.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "qwt_legend.h"
#include "qwt_plot_layout.h"
#include "qwt_plot_renderer.h"
#include "qwt_scale_draw.h"

#include <QDebug>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>
#include <QPagedPaintDevice>
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
        QList<RiuMultiPlotPage*> pages     = this->findChildren<RiuMultiPlotPage*>();
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
        QList<RiuMultiPlotPage*> pages = this->findChildren<RiuMultiPlotPage*>();
        QSize                    fullSize( 0, 0 );
        for ( auto page : pages )
        {
            fullSize.setWidth( std::max( fullSize.width(), page->minimumSizeHint().width() ) );
            fullSize.setHeight( fullSize.height() + 8 + page->minimumSizeHint().height() );
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
{
    const int spacing = 8;

    this->setBackgroundRole( QPalette::Dark );
    this->setContentsMargins( 0, 0, 0, 0 );
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
    m_book->setBackgroundRole( QPalette::Dark );
    m_bookLayout = new QVBoxLayout( m_book );
    m_bookLayout->setSpacing( spacing );
    m_scrollArea->setVisible( true );
    m_book->setVisible( true );

    setAutoFillBackground( true );

    this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    setFocusPolicy( Qt::StrongFocus );

    RiaApplication* app             = RiaApplication::instance();
    int             defaultFontSize = app->preferences()->defaultPlotFontSize();
    setFontSize( defaultFontSize );

    QSize pageSize = m_plotDefinition->pageLayout().fullRectPixels( RiaGuiApplication::applicationResolution() ).size();
    setBookSize( pageSize.width() );
    this->setObjectName( QString( "%1" ).arg( reinterpret_cast<uint64_t>( this ) ) );
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
void RiuMultiPlotBook::addPlot( RiuQwtPlotWidget* plotWidget )
{
    // Push the plot to the back of the list
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::insertPlot( RiuQwtPlotWidget* plotWidget, size_t index )
{
    m_plotWidgets.insert( static_cast<int>( index ), plotWidget );
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::removePlot( RiuQwtPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    int plotWidgetIdx = m_plotWidgets.indexOf( plotWidget );
    CVF_ASSERT( plotWidgetIdx >= 0 );

    m_plotWidgets.removeAt( plotWidgetIdx );

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle = plotTitle;
    for ( int i = 0; i < m_pages.size(); ++i )
    {
        m_pages[i]->setPlotTitle( QString( "%1 %2/%3" ).arg( m_plotTitle ).arg( i + 1 ).arg( m_pages.size() ) );
    }
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
void RiuMultiPlotBook::setFontSize( int fontSize )
{
    QFont font      = this->font();
    int   pixelSize = RiaFontCache::pointSizeToPixelSize( fontSize );

    font.setPixelSize( pixelSize );
    this->setFont( font );

    for ( auto page : m_pages )
    {
        page->setFontSize( fontSize );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotBook::fontSize() const
{
    return RiaFontCache::pixelSizeToPointSize( this->font().pixelSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotBook::indexOfPlotWidget( RiuQwtPlotWidget* plotWidget )
{
    return m_plotWidgets.indexOf( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotBook::previewModeEnabled() const
{
    return m_previewMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setPreviewModeEnabled( bool previewMode )
{
    m_previewMode = previewMode;

    for ( auto page : m_pages )
    {
        page->setPreviewModeEnabled( previewModeEnabled() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::scheduleUpdate()
{
    RiaPlotWindowRedrawScheduler::instance()->scheduleMultiPlotWindowUpdate( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::scheduleReplotOfAllPlots()
{
    for ( RiuQwtPlotWidget* plotWidget : visiblePlotWidgets() )
    {
        plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::renderTo( QPaintDevice* paintDevice )
{
    for ( auto page : m_pages )
    {
        page->stashWidgetStates();
    }

    int    resolution = paintDevice->logicalDpiX();
    double scaling    = resolution / static_cast<double>( RiaGuiApplication::applicationResolution() );

    bool     firstPage = true;
    QPainter painter( paintDevice );
    for ( RiuMultiPlotPage* page : m_pages )
    {
        if ( !firstPage )
        {
            QPagedPaintDevice* pagedDevice = dynamic_cast<QPagedPaintDevice*>( paintDevice );
            if ( pagedDevice )
            {
                pagedDevice->newPage();
            }
        }
        page->renderTo( &painter, scaling );
        firstPage = false;
    }

    for ( auto page : m_pages )
    {
        page->restoreWidgetStates();
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

    if ( menu.actions().size() > 0 )
    {
        menu.exec( event->globalPos() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    performUpdate();
    setBookSize( width() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::resizeEvent( QResizeEvent* event )
{
    setBookSize( event->size().width() );
    QWidget::resizeEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotBook::setBookSize( int frameWidth )
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
void RiuMultiPlotBook::performUpdate()
{
    deleteAllPages();
    createPages();
    updateGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuQwtPlotWidget>> RiuMultiPlotBook::visiblePlotWidgets() const
{
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets;
    for ( QPointer<RiuQwtPlotWidget> plotWidget : m_plotWidgets )
    {
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

    QList<QPointer<RiuQwtPlotWidget>> plotWidgets       = this->visiblePlotWidgets();
    auto                              rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

    int rowsPerPage = m_plotDefinition->rowsPerPage();
    int row         = 0;
    int column      = 0;

    // Make sure we always add a page. For empty multi-plots we'll have an empty page with a drop target.
    RiuMultiPlotPage* page = createPage();

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int expextedColSpan = static_cast<int>( plotWidgets[visibleIndex]->colSpan() );
        int colSpan         = std::min( expextedColSpan, rowAndColumnCount.second );

        std::tie( row, column ) = page->findAvailableRowAndColumn( row, column, colSpan, rowAndColumnCount.second );
        if ( row >= rowsPerPage )
        {
            page   = createPage();
            row    = 0;
            column = 0;
        }
        page->addPlot( plotWidgets[visibleIndex] );
        page->performUpdate();
    }
    m_book->adjustSize();
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

    // Reapply plot settings
    size_t pageNumber = m_pages.size() + 1;
    page->setPlotTitle( QString( "%1 %2/%3" ).arg( m_plotTitle ).arg( pageNumber ).arg( pageNumber ) );
    page->setFontSize( RiaApplication::instance()->preferences()->defaultPlotFontSize() );
    page->setTitleVisible( m_titleVisible );
    page->setSubTitlesVisible( m_subTitlesVisible );
    page->setPreviewModeEnabled( m_previewMode );

    m_pages.push_back( page );
    m_bookLayout->addWidget( page );

    page->setVisible( true );
    return page;
}
