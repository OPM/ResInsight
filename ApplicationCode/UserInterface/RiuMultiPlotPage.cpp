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

#include "RiuMultiPlotPage.h"

#include "RiaApplication.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "WellLogCommands/RicWellLogPlotTrackFeatureImpl.h"

#include "RiaGuiApplication.h"

#include "RimContextCommandBuilder.h"
#include "RimMultiPlotWindow.h"
#include "RimWellLogTrack.h"

#include "RiuMainWindow.h"
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
#include <QDesktopWidget>
#include <QFocusEvent>
#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>
#include <QScrollBar>
#include <QTimer>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotPage::RiuMultiPlotPage( RimMultiPlotWindow* plotDefinition, QWidget* parent )
    : RiuMultiPlotInterface( parent )
    , m_plotDefinition( plotDefinition )
{
    Q_ASSERT( plotDefinition );
    m_plotDefinition = plotDefinition;

    m_layout = new QVBoxLayout( this );
    m_layout->setMargin( 0 );
    m_layout->setSpacing( 2 );

    m_plotTitle = createTitleLabel();
    m_layout->addWidget( m_plotTitle );

    m_plotLayout = new QHBoxLayout;
    m_layout->addLayout( m_plotLayout );

    m_plotWidgetFrame = new QFrame;
    m_plotWidgetFrame->setVisible( true );
    m_plotLayout->addWidget( m_plotWidgetFrame, 1 );

    m_gridLayout = new QGridLayout( m_plotWidgetFrame );
    m_gridLayout->setContentsMargins( 0, 0, 0, 0 );
    m_gridLayout->setSpacing( 1 );

    QPalette newPalette( palette() );
    newPalette.setColor( QPalette::Window, Qt::white );
    setPalette( newPalette );

    setAutoFillBackground( true );

    new RiuPlotObjectPicker( m_plotTitle, m_plotDefinition );

    m_dropTargetPlaceHolder = new QLabel( "Drag plots here" );
    m_dropTargetPlaceHolder->setAlignment( Qt::AlignCenter );
    m_dropTargetPlaceHolder->setObjectName(
        QString( "%1" ).arg( reinterpret_cast<uint64_t>( m_dropTargetPlaceHolder.data() ) ) );
    m_dropTargetStyleSheet = createDropTargetStyleSheet();
    m_dropTargetStyleSheet.applyToWidget( m_dropTargetPlaceHolder );

    this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    setFocusPolicy( Qt::StrongFocus );

    setAcceptDrops( m_plotDefinition->acceptDrops() );

    RiaApplication* app = RiaApplication::instance();
    int defaultFontSize = RiaFontCache::pointSizeFromFontSizeEnum( app->preferences()->defaultPlotFontSize() );
    setFontSize( defaultFontSize );

    this->setObjectName( QString( "%1" ).arg( reinterpret_cast<uint64_t>( this ) ) );

    this->setBackgroundRole( QPalette::Window );

    QGraphicsDropShadowEffect* dropShadowEffect = new QGraphicsDropShadowEffect( this );
    dropShadowEffect->setOffset( 4.0, 4.0 );
    dropShadowEffect->setBlurRadius( 4.0 );
    dropShadowEffect->setColor( QColor( 40, 40, 40, 40 ) );
    this->setGraphicsEffect( dropShadowEffect );

    updateMarginsFromPageLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotPage::~RiuMultiPlotPage() {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMultiPlotWindow* RiuMultiPlotPage::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::addPlot( RiuQwtPlotWidget* plotWidget )
{
    // Insert the plot to the left of the scroll bar
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::insertPlot( RiuQwtPlotWidget* plotWidget, size_t index )
{
    plotWidget->setDraggable( true ); // Becomes draggable when added to a grid plot window
    m_plotWidgets.insert( static_cast<int>( index ), plotWidget );
    plotWidget->setVisible( false );

    QLabel* subTitle = new QLabel( plotWidget->plotDefinition()->description() );
    subTitle->setAlignment( Qt::AlignRight );
    subTitle->setVisible( false );
    m_subTitles.insert( static_cast<int>( index ), subTitle );

    RiuQwtPlotLegend* legend        = new RiuQwtPlotLegend( this );
    int               legendColumns = 1;
    if ( m_plotDefinition->legendsHorizontal() )
    {
        legendColumns = 4; // unlimited
    }
    legend->setMaxColumns( legendColumns );
    legend->horizontalScrollBar()->setVisible( false );
    legend->verticalScrollBar()->setVisible( false );
    legend->connect( plotWidget,
                     SIGNAL( legendDataChanged( const QVariant&, const QList<QwtLegendData>& ) ),
                     SLOT( updateLegend( const QVariant&, const QList<QwtLegendData>& ) ) );
    QObject::connect( legend, SIGNAL( legendUpdated() ), this, SLOT( onLegendUpdated() ) );

    legend->contentsWidget()->layout()->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
    legend->setVisible( false );
    plotWidget->updateLegend();
    m_legends.insert( static_cast<int>( index ), legend );

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::removePlot( RiuQwtPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    int plotWidgetIdx = m_plotWidgets.indexOf( plotWidget );
    if ( plotWidgetIdx < 0 ) return;

    m_plotWidgets.removeAt( plotWidgetIdx );
    plotWidget->setParent( nullptr );

    RiuQwtPlotLegend* legend = m_legends[plotWidgetIdx];
    legend->setParent( nullptr );
    m_legends.removeAt( plotWidgetIdx );
    delete legend;

    QLabel* subTitle = m_subTitles[plotWidgetIdx];
    subTitle->setParent( nullptr );
    m_subTitles.removeAt( plotWidgetIdx );
    delete subTitle;

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::removeAllPlots()
{
    auto plotWidgets = m_plotWidgets;
    for ( RiuQwtPlotWidget* plotWidget : plotWidgets )
    {
        removePlot( plotWidget );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle->setText( plotTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setTitleVisible( bool visible )
{
    m_plotTitle->setVisible( visible );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::setSelectionsVisible( bool visible )
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
void RiuMultiPlotPage::setFontSize( int fontSize )
{
    QFont font = m_plotTitle->font();

    font.setPointSize( fontSize + 1 );
    font.setBold( true );
    m_plotTitle->setFont( font );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::fontSize() const
{
    return m_plotTitle->font().pointSize() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::indexOfPlotWidget( RiuQwtPlotWidget* plotWidget )
{
    return m_plotWidgets.indexOf( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::scheduleUpdate()
{
    RiaPlotWindowRedrawScheduler::instance()->scheduleMultiPlotPageUpdate( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::scheduleReplotOfAllPlots()
{
    for ( RiuQwtPlotWidget* plotWidget : visiblePlotWidgets() )
    {
        plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::renderTo( QPaintDevice* paintDevice )
{
    QPainter painter( paintDevice );
    renderTo( &painter );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::renderTo( QPainter* painter )
{
    setSelectionsVisible( false );
    m_plotTitle->render( painter );

    // Subtract margins because we are rendering into part inside the margins
    QPoint marginOffset( m_layout->contentsMargins().left(), m_layout->contentsMargins().top() );

    for ( auto subTitle : subTitlesForVisiblePlots() )
    {
        if ( subTitle->isVisible() )
        {
            QPoint renderOffset = m_plotWidgetFrame->mapToParent( subTitle->frameGeometry().topLeft() ) - marginOffset;
            subTitle->render( painter, renderOffset );
        }
    }

    for ( auto legend : legendsForVisiblePlots() )
    {
        QPoint renderOffset = m_plotWidgetFrame->mapToParent( legend->frameGeometry().topLeft() ) - marginOffset;
        legend->render( painter, renderOffset );
    }

    for ( auto plotWidget : visiblePlotWidgets() )
    {
        QRect  plotWidgetGeometry     = plotWidget->frameGeometry();
        QPoint plotWidgetTopLeft      = plotWidgetGeometry.topLeft();
        QPoint plotWidgetFrameTopLeft = m_plotWidgetFrame->frameGeometry().topLeft();
        plotWidgetGeometry.moveTo( plotWidgetTopLeft + plotWidgetFrameTopLeft - marginOffset );

        plotWidget->renderTo( painter, plotWidgetGeometry );
    }

    setSelectionsVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::contextMenuEvent( QContextMenuEvent* event )
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
QLabel* RiuMultiPlotPage::createTitleLabel() const
{
    QLabel* plotTitle = new QLabel( "PLOT TITLE HERE", nullptr );
    plotTitle->setVisible( m_plotDefinition->isMultiPlotTitleVisible() );
    plotTitle->setAlignment( Qt::AlignHCenter );
    plotTitle->setWordWrap( true );
    plotTitle->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    return plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::dragEnterEvent( QDragEnterEvent* event )
{
    RiuQwtPlotWidget* source = dynamic_cast<RiuQwtPlotWidget*>( event->source() );
    if ( source )
    {
        setWidgetState( "dragTargetInto" );
        event->acceptProposedAction();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::dragMoveEvent( QDragMoveEvent* event )
{
    if ( event->answerRect().intersects( this->geometry() ) )
    {
        RiuQwtPlotWidget* source = dynamic_cast<RiuQwtPlotWidget*>( event->source() );
        if ( source && willAcceptDroppedPlot( source ) )
        {
            setWidgetState( "dragTargetInto" );

            QRect  originalGeometry = source->geometry();
            QPoint offset           = source->dragStartPosition();
            QRect  newRect( event->pos() - offset, originalGeometry.size() );

            QList<QPointer<RiuQwtPlotWidget>> visiblePlotWidgets = this->visiblePlotWidgets();

            int insertBeforeIndex = visiblePlotWidgets.size();
            for ( int visibleIndex = 0; visibleIndex < visiblePlotWidgets.size(); ++visibleIndex )
            {
                caf::UiStyleSheet::clearWidgetStates( visiblePlotWidgets[visibleIndex] );

                if ( visiblePlotWidgets[visibleIndex]->frameIsInFrontOfThis( newRect ) )
                {
                    insertBeforeIndex = std::min( insertBeforeIndex, visibleIndex );
                }
            }
            if ( insertBeforeIndex >= 0 && insertBeforeIndex < visiblePlotWidgets.size() )
            {
                visiblePlotWidgets[insertBeforeIndex]->setWidgetState( "dragTargetBefore" );
            }

            if ( insertBeforeIndex > 0 )
            {
                int insertAfterIndex = insertBeforeIndex - 1;
                visiblePlotWidgets[insertAfterIndex]->setWidgetState( "dragTargetAfter" );
            }
            event->acceptProposedAction();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::dragLeaveEvent( QDragLeaveEvent* event )
{
    caf::UiStyleSheet::clearWidgetStates( this );

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        caf::UiStyleSheet::clearWidgetStates( m_plotWidgets[tIdx] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::dropEvent( QDropEvent* event )
{
    caf::UiStyleSheet::clearWidgetStates( this );

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        caf::UiStyleSheet::clearWidgetStates( m_plotWidgets[tIdx] );
    }

    if ( this->geometry().contains( event->pos() ) )
    {
        RiuQwtPlotWidget* source = dynamic_cast<RiuQwtPlotWidget*>( event->source() );

        if ( source && willAcceptDroppedPlot( source ) )
        {
            event->acceptProposedAction();

            QRect  originalGeometry = source->geometry();
            QPoint offset           = source->dragStartPosition();
            QRect  newRect( event->pos() - offset, originalGeometry.size() );

            int beforeIndex = m_plotWidgets.size();
            for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
            {
                if ( m_plotWidgets[tIdx]->isVisible() )
                {
                    if ( m_plotWidgets[tIdx]->frameIsInFrontOfThis( newRect ) )
                    {
                        beforeIndex = tIdx;
                        break;
                    }
                }
            }
            RimPlot* insertAfter = nullptr;
            if ( beforeIndex > 0 )
            {
                insertAfter = m_plotWidgets[beforeIndex - 1]->plotDefinition();
            }

            RimPlot* plotToMove = source->plotDefinition();

            if ( insertAfter != plotToMove )
            {
                m_plotDefinition->movePlotsToThis( {plotToMove}, insertAfter );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotPage::hasHeightForWidth() const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::heightForWidth( int width ) const
{
    QPageLayout pageLayout  = m_plotDefinition->pageLayout();
    QRectF      rect        = pageLayout.fullRectPoints();
    double      aspectRatio = rect.height() / rect.width();
    int         height      = static_cast<int>( width * aspectRatio );
    return height;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::updateMarginsFromPageLayout()
{
    QPageLayout pageLayout = m_plotDefinition->pageLayout();
    const int   resolution = RiaGuiApplication::applicationResolution();
    QMargins    margins    = pageLayout.marginsPixels( resolution );
    m_layout->setContentsMargins( margins );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMultiPlotPage::sizeHint() const
{
    QPageLayout pageLayout = m_plotDefinition->pageLayout();
    const int   resolution = RiaGuiApplication::applicationResolution();
    QRect       rect       = pageLayout.fullRectPixels( resolution );

    return rect.size();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuMultiPlotPage::minimumSizeHint() const
{
    QSize fullSizeHint = sizeHint();
    return QSize( fullSizeHint.width() / 2, fullSizeHint.height() / 2 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotPage::willAcceptDroppedPlot( const RiuQwtPlotWidget* plotWidget ) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RiuMultiPlotPage::rowAndColumnCount( int plotWidgetCount ) const
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
void RiuMultiPlotPage::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    for ( RiuQwtPlotWidget* plotWidget : m_plotWidgets )
    {
        CAF_ASSERT( plotWidget );

        bool isSelected = false;
        for ( int changedLevel : changedSelectionLevels )
        {
            isSelected = isSelected ||
                         caf::SelectionManager::instance()->isSelected( plotWidget->plotDefinition(), changedLevel );
        }
        if ( isSelected )
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
void RiuMultiPlotPage::setWidgetState( const QString& widgetState )
{
    m_dropTargetStyleSheet.setWidgetState( m_dropTargetPlaceHolder, widgetState );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotPage::showYAxis( int row, int column ) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::performUpdate()
{
    updateMarginsFromPageLayout();
    reinsertPlotWidgets();
    alignCanvasTops();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::onLegendUpdated()
{
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::reinsertPlotWidgets()
{
    clearGridLayout();

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        if ( m_plotWidgets[tIdx] )
        {
            m_plotWidgets[tIdx]->hide();
        }
        if ( m_legends[tIdx] )
        {
            m_legends[tIdx]->hide();
        }
        if ( m_subTitles[tIdx] )
        {
            m_subTitles[tIdx]->hide();
        }
    }

    QList<QPointer<QLabel>>           subTitles   = this->subTitlesForVisiblePlots();
    QList<QPointer<RiuQwtPlotLegend>> legends     = this->legendsForVisiblePlots();
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = this->visiblePlotWidgets();

    if ( plotWidgets.empty() && acceptDrops() )
    {
        m_gridLayout->addWidget( m_dropTargetPlaceHolder, 0, 0 );
        m_gridLayout->setRowStretch( 0, 1 );
        m_dropTargetPlaceHolder->setVisible( true );
    }
    else
    {
        m_dropTargetPlaceHolder->setVisible( false );

        auto rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

        int row    = 0;
        int column = 0;
        for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
        {
            int expextedColSpan = static_cast<int>( plotWidgets[visibleIndex]->plotDefinition()->colSpan() );
            int colSpan         = std::min( expextedColSpan, rowAndColumnCount.second );
            int rowSpan         = plotWidgets[visibleIndex]->plotDefinition()->rowSpan();

            std::tie( row, column ) = findAvailableRowAndColumn( row, column, colSpan, rowAndColumnCount.second );

            m_gridLayout->addWidget( subTitles[visibleIndex], 3 * row, column, 1, colSpan );
            m_gridLayout
                ->addWidget( legends[visibleIndex], 3 * row + 1, column, 1, colSpan, Qt::AlignHCenter | Qt::AlignBottom );
            m_gridLayout->addWidget( plotWidgets[visibleIndex], 3 * row + 2, column, 1 + ( rowSpan - 1 ) * 3, colSpan );

            subTitles[visibleIndex]->setVisible( m_plotDefinition->showPlotTitles() );

            plotWidgets[visibleIndex]->setAxisLabelsAndTicksEnabled( QwtPlot::yLeft, showYAxis( row, column ) );
            plotWidgets[visibleIndex]->setAxisTitleEnabled( QwtPlot::yLeft, showYAxis( row, column ) );

            plotWidgets[visibleIndex]->show();

            if ( m_plotDefinition->legendsVisible() )
            {
                int legendColumns = 1;
                if ( m_plotDefinition->legendsHorizontal() )
                {
                    legendColumns = 4; // unlimited
                }
                legends[visibleIndex]->setMaxColumns( legendColumns );
                QFont legendFont = legends[visibleIndex]->font();
                legendFont.setPointSize( m_plotDefinition->legendFontSize() );
                legends[visibleIndex]->setFont( legendFont );
                legends[visibleIndex]->show();
            }
            else
            {
                legends[visibleIndex]->hide();
            }

            // Set basic row and column stretches
            for ( int r = row; r < row + rowSpan; ++r )
            {
                m_gridLayout->setRowStretch( 3 * r + 2, 1 );
            }
            for ( int c = column; c < column + colSpan; ++c )
            {
                int colStretch = 1;
                if ( showYAxis( row, column ) ) colStretch += 1;
                m_gridLayout->setColumnStretch( c, std::max( colStretch, m_gridLayout->columnStretch( c ) ) );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotPage::alignCanvasTops()
{
    CVF_ASSERT( m_legends.size() == m_plotWidgets.size() );

    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = visiblePlotWidgets();
    QList<QPointer<RiuQwtPlotLegend>> legends     = legendsForVisiblePlots();
    if ( plotWidgets.empty() ) return 0;

    auto rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

    std::vector<double> maxExtents( rowAndColumnCount.first, 0.0 );

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int row = visibleIndex / rowAndColumnCount.second;

        QFont font      = m_plotWidgets[visibleIndex]->axisFont( QwtPlot::xTop );
        maxExtents[row] = std::max( maxExtents[row],
                                    plotWidgets[visibleIndex]->axisScaleDraw( QwtPlot::xTop )->extent( font ) );
    }

    for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
    {
        int row = visibleIndex / rowAndColumnCount.second;
        plotWidgets[visibleIndex]->axisScaleDraw( QwtPlot::xTop )->setMinimumExtent( maxExtents[row] );
        legends[visibleIndex]->adjustSize();
    }
    return maxExtents[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotPage::clearGridLayout()
{
    if ( m_gridLayout )
    {
        for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
        {
            m_gridLayout->removeWidget( m_subTitles[tIdx] );
            m_gridLayout->removeWidget( m_legends[tIdx] );
            m_gridLayout->removeWidget( m_plotWidgets[tIdx] );
        }

        QLayoutItem* item;
        while ( ( item = m_gridLayout->takeAt( 0 ) ) != nullptr )
        {
        }
        QWidget().setLayout( m_gridLayout );
        delete m_gridLayout;
        m_gridLayout = new QGridLayout( m_plotWidgetFrame );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::UiStyleSheet RiuMultiPlotPage::createDropTargetStyleSheet()
{
    caf::UiStyleSheet styleSheet;

    styleSheet.set( "background-color", "white" );
    styleSheet.set( "border", "1px dashed black" );
    styleSheet.set( "font-size", "14pt" );
    styleSheet.property( "dragTargetInto" ).set( "border", "1px dashed lime" );
    styleSheet.property( "dragTargetInto" ).set( "background-color", "#DDFFDD" );

    return styleSheet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuQwtPlotWidget>> RiuMultiPlotPage::visiblePlotWidgets() const
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
QList<QPointer<RiuQwtPlotLegend>> RiuMultiPlotPage::legendsForVisiblePlots() const
{
    QList<QPointer<RiuQwtPlotLegend>> legends;
    for ( int i = 0; i < m_plotWidgets.size(); ++i )
    {
        if ( m_plotWidgets[i]->isChecked() )
        {
            legends.push_back( m_legends[i] );
        }
    }
    return legends;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<QLabel>> RiuMultiPlotPage::subTitlesForVisiblePlots() const
{
    QList<QPointer<QLabel>> subTitles;
    for ( int i = 0; i < m_plotWidgets.size(); ++i )
    {
        if ( m_plotWidgets[i]->isChecked() )
        {
            subTitles.push_back( m_subTitles[i] );
        }
    }
    return subTitles;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int>
    RiuMultiPlotPage::findAvailableRowAndColumn( int startRow, int startColumn, int columnSpan, int columnCount ) const
{
    int availableRow    = startRow;
    int availableColumn = startColumn;
    while ( true )
    {
        for ( ; availableColumn < columnCount; ++availableColumn )
        {
            bool fits = true;
            for ( int c = availableColumn; ( c < availableColumn + columnSpan ) && fits; ++c )
            {
                if ( c >= columnCount )
                {
                    fits = false;
                }

                if ( m_gridLayout->itemAtPosition( 3 * availableRow, c ) != nullptr )
                {
                    fits = false;
                }
            }
            if ( fits )
            {
                return std::make_pair( availableRow, availableColumn );
            }
        }
        availableColumn = 0;
        availableRow++;
    }
    return std::make_pair( availableRow, availableColumn );
}
