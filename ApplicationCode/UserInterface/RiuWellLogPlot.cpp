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

#include "RiuWellLogPlot.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "WellLogCommands/RicWellLogPlotTrackFeatureImpl.h"

#include "RimContextCommandBuilder.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RiuMainWindow.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotObjectPicker.h"
#include "RiuWellLogTrack.h"

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
RiuWellLogPlot::RiuWellLogPlot( RimWellLogPlot* plotDefinition, QWidget* parent )
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

    m_trackFrame = new QFrame;
    m_trackFrame->setVisible( true );
    m_plotLayout->addWidget( m_trackFrame, 1 );

    m_trackLayout = new QGridLayout( m_trackFrame );
    m_trackLayout->setMargin( 0 );
    m_trackLayout->setSpacing( 2 );

    QPalette newPalette( palette() );
    newPalette.setColor( QPalette::Background, Qt::white );
    setPalette( newPalette );

    setAutoFillBackground( true );

    m_scrollBar = new QScrollBar( nullptr );
    m_scrollBar->setOrientation( Qt::Vertical );
    m_scrollBar->setSizePolicy( QSizePolicy::Fixed, QSizePolicy::Preferred );

    m_scrollBarLayout = new QVBoxLayout;
    m_scrollBarLayout->addWidget( m_scrollBar, 0 );

    new RiuPlotObjectPicker( m_plotTitle, m_plotDefinition );

    this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

    setFocusPolicy( Qt::StrongFocus );
    connect( m_scrollBar, SIGNAL( valueChanged( int ) ), this, SLOT( slotSetMinDepth( int ) ) );

    setAcceptDrops( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogPlot::~RiuWellLogPlot()
{
    if ( m_plotDefinition )
    {
        m_plotDefinition->detachAllCurves();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::addTrackPlot( RiuWellLogTrack* trackPlot )
{
    // Insert the plot to the left of the scroll bar
    insertTrackPlot( trackPlot, m_trackPlots.size() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::insertTrackPlot( RiuWellLogTrack* trackPlot, size_t index, bool updateLayoutAfter )
{
    m_trackPlots.insert( static_cast<int>( index ), trackPlot );

    QwtLegend* legend        = new QwtLegend( nullptr );
    int        legendColumns = 1;
    if ( m_plotDefinition->areTrackLegendsHorizontal() )
    {
        legendColumns = 0; // unlimited
    }
    legend->setMaxColumns( legendColumns );
    legend->horizontalScrollBar()->setVisible( false );
    legend->verticalScrollBar()->setVisible( false );

    legend->connect( trackPlot,
                     SIGNAL( legendDataChanged( const QVariant&, const QList<QwtLegendData>& ) ),
                     SLOT( updateLegend( const QVariant&, const QList<QwtLegendData>& ) ) );
    legend->contentsWidget()->layout()->setAlignment( Qt::AlignBottom | Qt::AlignHCenter );
    trackPlot->updateLegend();
    m_legends.insert( static_cast<int>( index ), legend );

    if ( updateLayoutAfter )
    {
        updateChildrenLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::removeTrackPlot( RiuWellLogTrack* trackPlot, bool updateLayoutAfter )
{
    if ( !trackPlot ) return;

    int trackIdx = m_trackPlots.indexOf( trackPlot );
    CVF_ASSERT( trackIdx >= 0 );

    m_trackPlots.removeAt( trackIdx );
    trackPlot->setParent( nullptr );

    QwtLegend* legend = m_legends[trackIdx];
    m_legends.removeAt( trackIdx );
    delete legend;

    if ( updateLayoutAfter )
    {
        updateChildrenLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setDepthZoomAndReplot( double minDepth, double maxDepth )
{
    for ( int tpIdx = 0; tpIdx < m_trackPlots.count(); tpIdx++ )
    {
        m_trackPlots[tpIdx]->setDepthZoom( minDepth, maxDepth );
        m_trackPlots[tpIdx]->replot();
    }

    updateScrollBar( minDepth, maxDepth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setPlotTitle( const QString& plotTitle )
{
    m_plotTitle->setText( plotTitle );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuWellLogPlot::preferredWidth() const
{
    int titleWidth = 0;
    if ( m_plotTitle && m_plotTitle->isVisible() )
    {
        titleWidth = m_plotTitle->width();
    }

    int sumTrackWidth = 0;
    for ( QPointer<RiuWellLogTrack> track : m_trackPlots )
    {
        if ( track->isVisible() )
        {
            sumTrackWidth += track->width();
        }
    }
    return std::max( titleWidth, sumTrackWidth );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setTitleVisible( bool visible )
{
    m_plotTitle->setVisible( visible );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setScrollbarVisible( bool visible )
{
    m_scrollBar->setVisible( visible );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RiuWellLogPlot::indexOfTrackPlot( RiuWellLogTrack* track )
{
    return m_trackPlots.indexOf( track );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateChildrenLayout()
{
    reinsertTracksAndScrollbar();
    alignCanvasTopsAndScrollbar();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::contextMenuEvent( QContextMenuEvent* event )
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
void RiuWellLogPlot::keyPressEvent( QKeyEvent* keyEvent )
{
    m_plotDefinition->handleKeyPressEvent( keyEvent );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLabel* RiuWellLogPlot::createTitleLabel() const
{
    QLabel* plotTitle = new QLabel( "PLOT TITLE HERE", nullptr );

    RiaApplication* app = RiaApplication::instance();

    QFont font            = plotTitle->font();
    int   defaultFontSize = RiaFontCache::pointSizeFromFontSizeEnum( app->preferences()->defaultPlotFontSize() );

    font.setPointSize( defaultFontSize + 1 );
    font.setBold( true );
    plotTitle->setFont( font );
    plotTitle->setVisible( m_plotDefinition->isPlotTitleVisible() );
    plotTitle->setAlignment( Qt::AlignHCenter );
    plotTitle->setWordWrap( true );
    plotTitle->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::Preferred );
    return plotTitle;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::resizeEvent( QResizeEvent* event )
{
    QWidget::resizeEvent( event );
    updateChildrenLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::showEvent( QShowEvent* event )
{
    QWidget::showEvent( event );
    updateChildrenLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::changeEvent( QEvent* event )
{
    QWidget::changeEvent( event );
    if ( event->type() == QEvent::WindowStateChange )
    {
        updateChildrenLayout();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::dragEnterEvent( QDragEnterEvent* event )
{
    if ( this->geometry().contains( event->pos() ) )
    {
        event->acceptProposedAction();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::dragMoveEvent( QDragMoveEvent* event )
{
    if ( this->geometry().contains( event->pos() ) )
    {
        RiuWellLogTrack* source = dynamic_cast<RiuWellLogTrack*>( event->source() );
        if ( source )
        {
            QRect  originalGeometry = source->geometry();
            QPoint offset           = source->dragStartPosition();
            QRect  newRect( event->pos() - offset, originalGeometry.size() );

            QList<QPointer<RiuWellLogTrack>> visibleTracks = this->visibleTracks();

            int insertBeforeIndex = visibleTracks.size();
            for ( int visibleIndex = 0; visibleIndex < visibleTracks.size(); ++visibleIndex )
            {
                visibleTracks[visibleIndex]->setDefaultStyleSheet();

                if ( visibleTracks[visibleIndex]->frameIsInFrontOfThis( newRect ) )
                {
                    insertBeforeIndex = std::min( insertBeforeIndex, visibleIndex );
                }
            }
            if ( insertBeforeIndex >= 0 && insertBeforeIndex < visibleTracks.size() )
            {
                visibleTracks[insertBeforeIndex]->setStyleSheetForThisObject(
                    "border-left: 2px solid red; border-top: none; border-bottom: none; border-right: none;" );
            }

            if ( insertBeforeIndex > 0 )
            {
                int insertAfterIndex = insertBeforeIndex - 1;
                visibleTracks[insertAfterIndex]->setStyleSheetForThisObject(
                    "border-left: none; border-top: none; border-bottom: none; border-right: 2px solid red;" );
            }
            event->acceptProposedAction();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::dragLeaveEvent( QDragLeaveEvent* event )
{
    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        m_trackPlots[tIdx]->setDefaultStyleSheet();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::dropEvent( QDropEvent* event )
{
    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        m_trackPlots[tIdx]->setDefaultStyleSheet();
    }

    if ( this->geometry().contains( event->pos() ) )
    {
        RiuWellLogTrack* source = dynamic_cast<RiuWellLogTrack*>( event->source() );

        if ( source )
        {
            event->acceptProposedAction();

            QRect  originalGeometry = source->geometry();
            QPoint offset           = source->dragStartPosition();
            QRect  newRect( event->pos() - offset, originalGeometry.size() );

            int beforeIndex = m_trackPlots.size();
            for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
            {
                if ( m_trackPlots[tIdx]->isVisible() )
                {
                    if ( m_trackPlots[tIdx]->frameIsInFrontOfThis( newRect ) )
                    {
                        beforeIndex = tIdx;
                        break;
                    }
                }
            }
            RimWellLogTrack* insertAfter = nullptr;
            if ( beforeIndex > 0 )
            {
                insertAfter = m_trackPlots[beforeIndex - 1]->plotDefinition();
            }

            RimWellLogTrack* rimTrack = source->plotDefinition();

            if ( insertAfter != rimTrack )
            {
                RicWellLogPlotTrackFeatureImpl::moveTracksToWellLogPlot( m_plotDefinition, {rimTrack}, insertAfter );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<int, int> RiuWellLogPlot::rowAndColumnCount( int trackCount ) const
{
    int columnCount = std::max( 1, std::min( m_plotDefinition->columnCount(), trackCount ) );
    int rowCount    = static_cast<int>( std::ceil( trackCount / static_cast<double>( columnCount ) ) );
    return std::make_pair( rowCount, columnCount );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateScrollBar( double minDepth, double maxDepth )
{
    double availableMinDepth;
    double availableMaxDepth;
    m_plotDefinition->availableDepthRange( &availableMinDepth, &availableMaxDepth );
    availableMaxDepth += 0.01 * ( availableMaxDepth - availableMinDepth );

    double visibleDepth = maxDepth - minDepth;

    m_scrollBar->blockSignals( true );
    {
        m_scrollBar->setRange( (int)availableMinDepth, (int)( ( availableMaxDepth - visibleDepth ) ) );
        m_scrollBar->setPageStep( (int)visibleDepth );
        m_scrollBar->setValue( (int)minDepth );
        m_scrollBar->setVisible( true );
    }
    m_scrollBar->blockSignals( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::alignCanvasTopsAndScrollbar()
{
    CVF_ASSERT( m_legends.size() == m_trackPlots.size() );

    QList<QPointer<RiuWellLogTrack>> tracks = visibleTracks();

    auto rowAndColumnCount = this->rowAndColumnCount( tracks.size() );

    std::vector<double> maxExtents( rowAndColumnCount.first, 0.0 );

    for ( int visibleIndex = 0; visibleIndex < tracks.size(); ++visibleIndex )
    {
        int row = visibleIndex / rowAndColumnCount.second;

        QFont font      = m_trackPlots[visibleIndex]->axisFont( QwtPlot::xTop );
        maxExtents[row] = std::max( maxExtents[row],
                                    tracks[visibleIndex]->axisScaleDraw( QwtPlot::xTop )->extent( font ) );
    }

    for ( int visibleIndex = 0; visibleIndex < tracks.size(); ++visibleIndex )
    {
        int row = visibleIndex / rowAndColumnCount.second;
        tracks[visibleIndex]->axisScaleDraw( QwtPlot::xTop )->setMinimumExtent( maxExtents[row] );
    }
    m_scrollBarLayout->setContentsMargins( 0, maxExtents[0], 0, 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::reinsertTracksAndScrollbar()
{
    clearTrackLayout();

    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        m_trackPlots[tIdx]->hide();
    }

    QList<QPointer<RiuWellLogTrack>> tracks  = this->visibleTracks();
    QList<QPointer<QwtLegend>>       legends = this->visibleLegends();

    auto rowAndColumnCount = this->rowAndColumnCount( tracks.size() );

    for ( int visibleIndex = 0; visibleIndex < tracks.size(); ++visibleIndex )
    {
        int row    = visibleIndex / rowAndColumnCount.second;
        int column = visibleIndex % rowAndColumnCount.second;

        m_trackLayout->addWidget( legends[visibleIndex], 2 * row, column );
        m_trackLayout->addWidget( tracks[visibleIndex], 2 * row + 1, column );

        if ( m_plotDefinition->areTrackLegendsVisible() )
        {
            int legendColumns = 1;
            if ( m_plotDefinition->areTrackLegendsHorizontal() )
            {
                legendColumns = 0; // unlimited
            }
            legends[visibleIndex]->setMaxColumns( legendColumns );
            int minimumHeight = legends[visibleIndex]->heightForWidth( tracks[visibleIndex]->width() );
            legends[visibleIndex]->setMinimumHeight( minimumHeight );

            legends[visibleIndex]->show();
        }
        else
        {
            legends[visibleIndex]->hide();
        }

        tracks[visibleIndex]->setDepthTitle( column == 0 ? m_plotDefinition->depthPlotTitle() : "" );
        tracks[visibleIndex]->enableDepthAxisLabelsAndTicks( column == 0 );
        tracks[visibleIndex]->show();

        int widthScaleFactor = tracks[visibleIndex]->widthScaleFactor();
        if ( column == 0 )
        {
            widthScaleFactor += 1; // Give it a bit extra room due to depth axis
        }
        m_trackLayout->setColumnStretch( column,
                                         std::max( m_trackLayout->columnStretch( column ),
                                                   tracks[visibleIndex]->widthScaleFactor() ) );
        m_trackLayout->setRowStretch( 2 * row + 1, 1 );
    }
    m_trackLayout->addLayout( m_scrollBarLayout, 1, rowAndColumnCount.second, rowAndColumnCount.first * 2 - 1, 1 );
    m_trackLayout->setColumnStretch( rowAndColumnCount.second, 0 );
    m_scrollBar->setVisible( tracks.size() > 0 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::clearTrackLayout()
{
    if ( m_trackLayout )
    {
        for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
        {
            m_trackLayout->removeWidget( m_legends[tIdx] );
            m_trackLayout->removeWidget( m_trackPlots[tIdx] );
        }

        QLayoutItem* item;
        while ( ( item = m_trackLayout->takeAt( 0 ) ) != 0 )
        {
        }
        QWidget().setLayout( m_trackLayout );
        delete m_trackLayout;
        m_trackLayout = new QGridLayout( m_trackFrame );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<RiuWellLogTrack>> RiuWellLogPlot::visibleTracks() const
{
    QList<QPointer<RiuWellLogTrack>> tracks;
    for ( QPointer<RiuWellLogTrack> track : m_trackPlots )
    {
        if ( track->isRimTrackVisible() )
        {
            tracks.push_back( track );
        }
    }
    return tracks;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QList<QPointer<QwtLegend>> RiuWellLogPlot::visibleLegends() const
{
    QList<QPointer<QwtLegend>> legends;
    for ( int i = 0; i < m_trackPlots.size(); ++i )
    {
        if ( m_trackPlots[i]->isRimTrackVisible() )
        {
            legends.push_back( m_legends[i] );
        }
    }
    return legends;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::slotSetMinDepth( int value )
{
    double minimumDepth;
    double maximumDepth;
    m_plotDefinition->depthZoomMinMax( &minimumDepth, &maximumDepth );

    double delta = value - minimumDepth;
    m_plotDefinition->setDepthZoomMinMax( minimumDepth + delta, maximumDepth + delta );
    m_plotDefinition->setDepthAutoZoom( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RiuWellLogPlot::ownerPlotDefinition()
{
    return m_plotDefinition;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimViewWindow* RiuWellLogPlot::ownerViewWindow() const
{
    return m_plotDefinition;
}
