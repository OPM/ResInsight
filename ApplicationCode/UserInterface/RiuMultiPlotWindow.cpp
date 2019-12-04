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

#include "RiuMultiPlotWindow.h"

#include "RiaApplication.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "WellLogCommands/RicWellLogPlotTrackFeatureImpl.h"

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
#include "qwt_scale_draw.h"

#include <QDebug>
#include <QFocusEvent>
#include <QHBoxLayout>
#include <QMdiSubWindow>
#include <QMenu>
#include <QScrollBar>
#include <QTimer>

#include <cmath>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuMultiPlotWindow::RiuMultiPlotWindow( RimMultiPlotWindow* plotDefinition, QWidget* parent )
    : QWidget( parent )
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
    m_gridLayout->setContentsMargins( 1, 1, 1, 1 );
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
    // Insert the plot to the left of the scroll bar
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::insertPlot( RiuQwtPlotWidget* plotWidget, size_t index )
{
    plotWidget->setDraggable( true ); // Becomes draggable when added to a grid plot window
    m_plotWidgets.insert( static_cast<int>( index ), plotWidget );

    QLabel* subTitle = new QLabel( plotWidget->plotDefinition()->description() );
    subTitle->setAlignment( Qt::AlignRight );
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
    plotWidget->updateLegend();
    m_legends.insert( static_cast<int>( index ), legend );

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
void RiuMultiPlotWindow::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle->setText( plotTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::setTitleVisible( bool visible )
{
    m_plotTitle->setVisible( visible );
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
    QFont font = m_plotTitle->font();

    font.setPointSize( fontSize + 1 );
    font.setBold( true );
    m_plotTitle->setFont( font );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuMultiPlotWindow::fontSize() const
{
    return m_plotTitle->font().pointSize() - 1;
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
void RiuMultiPlotWindow::scheduleUpdate()
{
    RiaPlotWindowRedrawScheduler::instance()->schedulePlotWindowUpdate( this );
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
QLabel* RiuMultiPlotWindow::createTitleLabel() const
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
void RiuMultiPlotWindow::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::dragEnterEvent( QDragEnterEvent* event )
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
void RiuMultiPlotWindow::dragMoveEvent( QDragMoveEvent* event )
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
void RiuMultiPlotWindow::dragLeaveEvent( QDragLeaveEvent* event )
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
void RiuMultiPlotWindow::dropEvent( QDropEvent* event )
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
                m_plotDefinition->movePlotsToThis( { plotToMove }, insertAfter );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuMultiPlotWindow::willAcceptDroppedPlot( const RiuQwtPlotWidget* plotWidget ) const
{
    return true;
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
void RiuMultiPlotWindow::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
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
void RiuMultiPlotWindow::setWidgetState( const QString& widgetState )
{
    m_dropTargetStyleSheet.setWidgetState( m_dropTargetPlaceHolder, widgetState );
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
    reinsertPlotWidgets();
    alignCanvasTops();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::onLegendUpdated()
{
    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuMultiPlotWindow::reinsertPlotWidgets()
{
    clearGridLayout();

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        m_plotWidgets[tIdx]->hide();
        m_legends[tIdx]->hide();
        m_subTitles[tIdx]->hide();
    }

    QList<QPointer<QLabel>>           subTitles   = this->visibleTitles();
    QList<QPointer<RiuQwtPlotLegend>> legends     = this->visibleLegends();
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = this->visiblePlotWidgets();

    if ( plotWidgets.empty() && acceptDrops() )
    {
        m_gridLayout->addWidget( m_dropTargetPlaceHolder, 0, 0 );
        m_gridLayout->setRowStretch( 0, 1 );
        m_dropTargetPlaceHolder->setVisible( false );
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
            m_gridLayout->addWidget( legends[visibleIndex], 3 * row + 1, column, 1, colSpan );
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
int RiuMultiPlotWindow::alignCanvasTops()
{
    CVF_ASSERT( m_legends.size() == m_plotWidgets.size() );

    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = visiblePlotWidgets();
    QList<QPointer<RiuQwtPlotLegend>> legends     = visibleLegends();
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
void RiuMultiPlotWindow::clearGridLayout()
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
caf::UiStyleSheet RiuMultiPlotWindow::createDropTargetStyleSheet()
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
QList<QPointer<RiuQwtPlotLegend>> RiuMultiPlotWindow::visibleLegends() const
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
QList<QPointer<QLabel>> RiuMultiPlotWindow::visibleTitles() const
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
    RiuMultiPlotWindow::findAvailableRowAndColumn( int startRow, int startColumn, int columnSpan, int columnCount ) const
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
