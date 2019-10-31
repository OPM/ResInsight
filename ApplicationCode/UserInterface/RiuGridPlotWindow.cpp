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

#include "RiuGridPlotWindow.h"

#include "RiaApplication.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaPreferences.h"

#include "WellLogCommands/RicWellLogPlotTrackFeatureImpl.h"

#include "RimContextCommandBuilder.h"
#include "RimGridPlotWindow.h"
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
RiuGridPlotWindow::RiuGridPlotWindow( RimGridPlotWindow* plotDefinition, QWidget* parent )
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
    newPalette.setColor( QPalette::Background, Qt::white );
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

    setAcceptDrops( true );

    RiaApplication* app = RiaApplication::instance();
    int defaultFontSize = RiaFontCache::pointSizeFromFontSizeEnum( app->preferences()->defaultPlotFontSize() );
    setFontSize( defaultFontSize );

    this->setObjectName( QString( "%1" ).arg( reinterpret_cast<uint64_t>( this ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGridPlotWindow::~RiuGridPlotWindow()
{
    if ( m_plotDefinition )
    {
        m_plotDefinition->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGridPlotWindow* RiuGridPlotWindow::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuGridPlotWindow::ownerViewWindow() const
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::addPlot( RiuQwtPlotWidget* plotWidget )
{
    // Insert the plot to the left of the scroll bar
    insertPlot( plotWidget, m_plotWidgets.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::insertPlot( RiuQwtPlotWidget* plotWidget, size_t index )
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
        legendColumns = 0; // unlimited
    }
    legend->setMaxColumns( legendColumns );
    legend->horizontalScrollBar()->setVisible( false );
    legend->verticalScrollBar()->setVisible( false );

    legend->connect( plotWidget,
                     SIGNAL( legendDataChanged( const QVariant&, const QList<QwtLegendData>& ) ),
                     SLOT( updateLegend( const QVariant&, const QList<QwtLegendData>& ) ) );

    legend->contentsWidget()->layout()->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
    plotWidget->updateLegend();
    m_legends.insert( static_cast<int>( index ), legend );
    m_legendColumns.insert( static_cast<int>( index ), -1 );

    scheduleUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::removePlot( RiuQwtPlotWidget* plotWidget )
{
    if ( !plotWidget ) return;

    int plotWidgetIdx = m_plotWidgets.indexOf( plotWidget );
    CVF_ASSERT( plotWidgetIdx >= 0 );

    m_plotWidgets.removeAt( plotWidgetIdx );
    plotWidget->setParent( nullptr );

    RiuQwtPlotLegend* legend = m_legends[plotWidgetIdx];
    legend->setParent( nullptr );
    m_legends.removeAt( plotWidgetIdx );
    m_legendColumns.removeAt( plotWidgetIdx );
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
void RiuGridPlotWindow::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle->setText( plotTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::setTitleVisible( bool visible )
{
    m_plotTitle->setVisible( visible );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::setSelectionsVisible( bool visible )
{
    for ( RiuQwtPlotWidget* plotWidget : m_plotWidgets )
    {
        if ( visible && caf::SelectionManager::instance()->isSelected( plotWidget->plotOwner(), 0 ) )
        {
            plotWidget->setWidgetState( RiuWidgetStyleSheet::SELECTED );
        }
        else
        {
            plotWidget->setWidgetState( RiuWidgetStyleSheet::DEFAULT );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::setFontSize( int fontSize )
{
    QFont font = m_plotTitle->font();

    font.setPointSize( fontSize + 1 );
    font.setBold( true );
    m_plotTitle->setFont( font );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuGridPlotWindow::fontSize() const
{
    return m_plotTitle->font().pointSize() - 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuGridPlotWindow::indexOfPlotWidget( RiuQwtPlotWidget* plotWidget )
{
    return m_plotWidgets.indexOf( plotWidget );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::scheduleUpdate()
{
    RiaPlotWindowRedrawScheduler::instance()->schedulePlotWindowUpdate( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::scheduleReplotOfAllPlots()
{
    for ( RiuQwtPlotWidget* plotWidget : visiblePlotWidgets() )
    {
        plotWidget->scheduleReplot();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::contextMenuEvent( QContextMenuEvent* event )
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
void RiuGridPlotWindow::keyPressEvent( QKeyEvent* keyEvent )
{
    m_plotDefinition->handleKeyPressEvent( keyEvent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLabel* RiuGridPlotWindow::createTitleLabel() const
{
    QLabel* plotTitle = new QLabel( "PLOT TITLE HERE", nullptr );
    plotTitle->setVisible( m_plotDefinition->isPlotTitleVisible() );
    plotTitle->setAlignment( Qt::AlignHCenter );
    plotTitle->setWordWrap( true );
    plotTitle->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    return plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent( event );
    bool needsUpdate = false;
    for ( int i = 0; i < m_legends.size(); ++i )
    {
        if ( m_legends[i]->isVisible() )
        {
            int columnCount = m_legends[i]->columnCount();
            if ( columnCount != m_legendColumns[i] )
            {
                int oldColumnCount = m_legendColumns[i];
                m_legendColumns[i] = columnCount;
                if ( oldColumnCount != -1 )
                {
                    needsUpdate = true;
                }
            }
        }
    }
    if ( needsUpdate )
    {
        performUpdate();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    performUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::dragEnterEvent( QDragEnterEvent* event )
{
    RiuQwtPlotWidget* source = dynamic_cast<RiuQwtPlotWidget*>( event->source() );
    if ( source )
    {
        setWidgetState( RiuWidgetStyleSheet::DRAG_TARGET_INTO );
        event->acceptProposedAction();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::dragMoveEvent( QDragMoveEvent* event )
{
    if ( event->answerRect().intersects( this->geometry() ) )
    {
        RiuQwtPlotWidget* source = dynamic_cast<RiuQwtPlotWidget*>( event->source() );
        if ( source && willAcceptDroppedPlot( source ) )
        {
            setWidgetState( RiuWidgetStyleSheet::DRAG_TARGET_INTO );

            QRect  originalGeometry = source->geometry();
            QPoint offset           = source->dragStartPosition();
            QRect  newRect( event->pos() - offset, originalGeometry.size() );

            QList<QPointer<RiuQwtPlotWidget>> visiblePlotWidgets = this->visiblePlotWidgets();

            int insertBeforeIndex = visiblePlotWidgets.size();
            for ( int visibleIndex = 0; visibleIndex < visiblePlotWidgets.size(); ++visibleIndex )
            {
                visiblePlotWidgets[visibleIndex]->setWidgetState( RiuWidgetStyleSheet::DEFAULT );

                if ( visiblePlotWidgets[visibleIndex]->frameIsInFrontOfThis( newRect ) )
                {
                    insertBeforeIndex = std::min( insertBeforeIndex, visibleIndex );
                }
            }
            if ( insertBeforeIndex >= 0 && insertBeforeIndex < visiblePlotWidgets.size() )
            {
                visiblePlotWidgets[insertBeforeIndex]->setWidgetState( RiuWidgetStyleSheet::DRAG_TARGET_BEFORE );
            }

            if ( insertBeforeIndex > 0 )
            {
                int insertAfterIndex = insertBeforeIndex - 1;
                visiblePlotWidgets[insertAfterIndex]->setWidgetState( RiuWidgetStyleSheet::DRAG_TARGET_AFTER );
            }
            event->acceptProposedAction();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::dragLeaveEvent( QDragLeaveEvent* event )
{
    setWidgetState( RiuWidgetStyleSheet::DEFAULT );

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        m_plotWidgets[tIdx]->setWidgetState( RiuWidgetStyleSheet::DEFAULT );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::dropEvent( QDropEvent* event )
{
    setWidgetState( RiuWidgetStyleSheet::DEFAULT );

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        m_plotWidgets[tIdx]->setWidgetState( RiuWidgetStyleSheet::DEFAULT );
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
            RimPlotInterface* insertAfter = nullptr;
            if ( beforeIndex > 0 )
            {
                insertAfter = m_plotWidgets[beforeIndex - 1]->plotDefinition();
            }

            RimPlotInterface* plotToMove = source->plotDefinition();

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
bool RiuGridPlotWindow::willAcceptDroppedPlot( const RiuQwtPlotWidget* plotWidget ) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RiuGridPlotWindow::rowAndColumnCount( int plotWidgetCount ) const
{
    if ( plotWidgetCount == 0 )
    {
        return std::make_pair( 0, 0 );
    }

    int columnCount = std::max( 1, std::min( m_plotDefinition->columnCount(), plotWidgetCount ) );
    int rowCount    = static_cast<int>( std::ceil( plotWidgetCount / static_cast<double>( columnCount ) ) );
    return std::make_pair( rowCount, columnCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::onSelectionManagerSelectionChanged( const std::set<int>& changedSelectionLevels )
{
    for ( RiuQwtPlotWidget* plotWidget : m_plotWidgets )
    {
        CAF_ASSERT( plotWidget );

        bool isSelected = false;
        for ( int changedLevel : changedSelectionLevels )
        {
            isSelected = isSelected ||
                         caf::SelectionManager::instance()->isSelected( plotWidget->plotOwner(), changedLevel );
        }
        if ( isSelected )
        {
            plotWidget->setWidgetState( RiuWidgetStyleSheet::SELECTED );
        }
        else
        {
            plotWidget->setWidgetState( RiuWidgetStyleSheet::DEFAULT );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::setWidgetState( RiuWidgetStyleSheet::StateTag widgetState )
{
    m_dropTargetStyleSheet.setWidgetState( m_dropTargetPlaceHolder, widgetState );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiuGridPlotWindow::showYAxis( int row, int column ) const
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::performUpdate()
{
    reinsertPlotWidgets();
    alignCanvasTops();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::reinsertPlotWidgets()
{
    clearGridLayout();

    for ( int tIdx = 0; tIdx < m_plotWidgets.size(); ++tIdx )
    {
        m_plotWidgets[tIdx]->hide();
        m_legends[tIdx]->hide();
    }

    QList<QPointer<QLabel>>           subTitles   = this->visibleTitles();
    QList<QPointer<RiuQwtPlotLegend>> legends     = this->visibleLegends();
    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = this->visiblePlotWidgets();

    if ( plotWidgets.empty() )
    {
        m_gridLayout->addWidget( m_dropTargetPlaceHolder, 0, 0 );
        m_gridLayout->setRowStretch( 0, 1 );
        m_dropTargetPlaceHolder->setVisible( true );
    }
    else
    {
        m_dropTargetPlaceHolder->setVisible( false );

        auto rowAndColumnCount = this->rowAndColumnCount( plotWidgets.size() );

        for ( int visibleIndex = 0; visibleIndex < plotWidgets.size(); ++visibleIndex )
        {
            int row    = visibleIndex / rowAndColumnCount.second;
            int column = visibleIndex % rowAndColumnCount.second;

            m_gridLayout->addWidget( subTitles[visibleIndex], 3 * row, column );
            m_gridLayout->addWidget( legends[visibleIndex], 3 * row + 1, column );
            m_gridLayout->addWidget( plotWidgets[visibleIndex], 3 * row + 2, column );

            subTitles[visibleIndex]->setVisible( m_plotDefinition->showPlotTitles() );

            if ( m_plotDefinition->legendsVisible() )
            {
                int legendColumns = 1;
                if ( m_plotDefinition->legendsHorizontal() )
                {
                    legendColumns = 0; // unlimited
                }
                legends[visibleIndex]->setMaxColumns( legendColumns );
                int minimumHeight = legends[visibleIndex]->heightForWidth( plotWidgets[visibleIndex]->width() );
                legends[visibleIndex]->setMinimumHeight( minimumHeight );
                QFont legendFont = legends[visibleIndex]->font();
                legendFont.setPointSize( m_plotDefinition->legendFontSize() );
                legends[visibleIndex]->setFont( legendFont );
                legends[visibleIndex]->show();
            }
            else
            {
                legends[visibleIndex]->hide();
            }

            plotWidgets[visibleIndex]->setAxisLabelsAndTicksEnabled( QwtPlot::yLeft, showYAxis( row, column ) );
            plotWidgets[visibleIndex]->setAxisTitleEnabled( QwtPlot::yLeft, showYAxis( row, column ) );

            plotWidgets[visibleIndex]->show();

            int widthScaleFactor = plotWidgets[visibleIndex]->widthScaleFactor();
            if ( showYAxis( row, column ) )
            {
                widthScaleFactor += 1; // Give it a bit extra room due to axis
            }
            m_gridLayout->setColumnStretch( column,
                                            std::max( m_gridLayout->columnStretch( column ),
                                                      plotWidgets[visibleIndex]->widthScaleFactor() ) );
            m_gridLayout->setRowStretch( 3 * row + 2, 1 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuGridPlotWindow::alignCanvasTops()
{
    CVF_ASSERT( m_legends.size() == m_plotWidgets.size() );

    QList<QPointer<RiuQwtPlotWidget>> plotWidgets = visiblePlotWidgets();
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
    }
    return maxExtents[0];
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGridPlotWindow::clearGridLayout()
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
        while ( ( item = m_gridLayout->takeAt( 0 ) ) != 0 )
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
RiuWidgetStyleSheet RiuGridPlotWindow::createDropTargetStyleSheet()
{
    RiuWidgetStyleSheet styleSheet;

    styleSheet.set( "background-color", "white" );
    styleSheet.set( "border", "1px dashed black" );
    styleSheet.set( "font-size", "14pt" );
    styleSheet.state( RiuWidgetStyleSheet::DRAG_TARGET_INTO ).set( "border", "1px dashed lime" );
    styleSheet.state( RiuWidgetStyleSheet::DRAG_TARGET_INTO ).set( "background-color", "#DDFFDD" );

    return styleSheet;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuQwtPlotWidget>> RiuGridPlotWindow::visiblePlotWidgets() const
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
QList<QPointer<RiuQwtPlotLegend>> RiuGridPlotWindow::visibleLegends() const
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
QList<QPointer<QLabel>> RiuGridPlotWindow::visibleTitles() const
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
