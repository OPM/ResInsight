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
#include "RiuMultiPlotWindow.h"

#include "RiaGuiApplication.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "RimContextCommandBuilder.h"
#include "RimMultiPlotWindow.h"
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
RiuMultiPlotWindow::RiuMultiPlotWindow( RimMultiPlotWindow* plotDefinition, QWidget* parent )
    : RiuMultiPlotInterface( parent )
    , m_plotDefinition( plotDefinition )
    , m_plotTitle( "Multi Plot" )
    , m_titleVisible( true )
    , m_previewMode( true )
{
    Q_ASSERT( plotDefinition );
    m_plotDefinition = plotDefinition;

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
RiuMultiPlotWindow::~RiuMultiPlotWindow() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotWindow* RiuMultiPlotWindow::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuMultiPlotWindow::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::addPlot( RiuQwtPlotWidget* plotWidget )
{
    // Push the plot to the back of the list
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::insertPlot( RiuQwtPlotWidget* plotWidget, size_t index )
{
    plotWidget->setDraggable( true ); // Becomes draggable when added to a grid plot window
    m_plotWidgets.insert( static_cast<int>( index ), plotWidget );
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::removePlot( RiuQwtPlotWidget* plotWidget )
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
void RiuMultiPlotWindow::setPlotTitle( const QString& plotTitle )
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
void RiuMultiPlotWindow::setTitleVisible( bool visible )
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
void RiuMultiPlotWindow::setSelectionsVisible( bool visible )
{
    for ( RiuQwtPlotWidget* plotWidget : m_plotWidgets )
    {
        if ( visible && caf::SelectionManager::instance()->isSelected( plotWidget->plotDefinition(), 0 ) )
        {
            plotWidget->setWidgetState( "selected" );
        }
        else
        {
            caf::UiStyleSheet::clearWidgetStates( plotWidget );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::setFontSize( int fontSize )
{
    QFont font      = this->font();
    int   pixelSize = RiaFontCache::pointSizeToPixelSize( fontSize );

    font.setPixelSize( pixelSize );
    this->setFont( font );

    for ( auto page : m_pages )
    {
        page->setFontSize( fontSize );
    }

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotWindow::fontSize() const
{
    return RiaFontCache::pixelSizeToPointSize( this->font().pixelSize() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotWindow::indexOfPlotWidget( RiuQwtPlotWidget* plotWidget )
{
    return m_plotWidgets.indexOf( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotWindow::previewModeEnabled() const
{
    return m_previewMode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::setPreviewModeEnabled( bool previewMode )
{
    m_previewMode = previewMode;

    for ( auto page : m_pages )
    {
        page->setPreviewModeEnabled( previewModeEnabled() );
    }

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::scheduleUpdate()
{
    RiaPlotWindowRedrawScheduler::instance()->scheduleMultiPlotWindowUpdate( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::scheduleReplotOfAllPlots()
{
    for ( RiuQwtPlotWidget* plotWidget : visiblePlotWidgets() )
    {
        plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::renderTo( QPaintDevice* paintDevice )
{
    setSelectionsVisible( false );

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

    setSelectionsVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::contextMenuEvent( QContextMenuEvent* event )
{
    QMenu                      menu;
    caf::CmdFeatureMenuBuilder menuBuilder;

    caf::SelectionManager::instance()->setSelectedItem( ownerPlotDefinition() );

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
void RiuMultiPlotWindow::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    performUpdate();
    setBookSize( width() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::resizeEvent( QResizeEvent* event )
{
    setBookSize( event->size().width() );
    QWidget::resizeEvent( event );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::setBookSize( int frameWidth )
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
std::pair<int, int> RiuMultiPlotWindow::rowAndColumnCount( int plotWidgetCount ) const
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
bool RiuMultiPlotWindow::showYAxis( int row, int column ) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::performUpdate()
{
    deleteAllPages();
    createPages();
    updateGeometry();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuQwtPlotWidget>> RiuMultiPlotWindow::visiblePlotWidgets() const
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
void RiuMultiPlotWindow::deleteAllPages()
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
void RiuMultiPlotWindow::createPages()
{
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets       = this->visiblePlotWidgets();
    auto                              rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

    int rowsPerPage = m_plotDefinition->rowsPerPage();
    int row         = 0;
    int column      = 0;

    RiuMultiPlotPage* page = new RiuMultiPlotPage( m_plotDefinition, this );
    m_pages.push_back( page );
    m_bookLayout->addWidget( page );
    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int expextedColSpan = static_cast<int>( plotWidgets[visibleIndex]->plotDefinition()->colSpan() );
        int colSpan         = std::min( expextedColSpan, rowAndColumnCount.second );

        std::tie( row, column ) = page->findAvailableRowAndColumn( row, column, colSpan, rowAndColumnCount.second );
        if ( row >= rowsPerPage )
        {
            page = new RiuMultiPlotPage( m_plotDefinition, this );

            m_pages.push_back( page );
            m_bookLayout->addWidget( page );
            row    = 0;
            column = 0;
        }
        page->addPlot( plotWidgets[visibleIndex] );
        page->setVisible( true );
        page->performUpdate();
    }
    // Reapply plot settings
    setPlotTitle( m_plotTitle );

    setFontSize( RiaApplication::instance()->preferences()->defaultPlotFontSize() );
    setTitleVisible( m_titleVisible );
    setPreviewModeEnabled( m_previewMode );
    m_book->adjustSize();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const QList<QPointer<RiuMultiPlotPage>>& RiuMultiPlotWindow::pages() const
{
    return m_pages;
}
