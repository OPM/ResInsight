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

    m_plotFrame = new QFrame;
    m_plotFrame->setVisible( true );
    m_plotLayout->addWidget( m_plotFrame, 1 );

    m_trackLayout = new QGridLayout( m_plotFrame );
    m_trackLayout->setMargin( 0 );
    m_trackLayout->setSpacing( 2 );

    QPalette newPalette( palette() );
    newPalette.setColor( QPalette::Background, Qt::white );
    setPalette( newPalette );

    setAutoFillBackground( true );

    m_scrollBarLayout = new QVBoxLayout;
    m_scrollBarLayout->setContentsMargins( 0, 50, 0, 0 );
    m_plotLayout->addLayout( m_scrollBarLayout );

    m_scrollBar = new QScrollBar( nullptr );
    m_scrollBar->setOrientation( Qt::Vertical );
    m_scrollBar->setVisible( true );

    m_scrollBarLayout->addWidget( m_scrollBar, 0 );

    new RiuPlotObjectPicker( m_plotTitle, m_plotDefinition );

    this->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding );

    setFocusPolicy( Qt::StrongFocus );
    connect( m_scrollBar, SIGNAL( valueChanged( int ) ), this, SLOT( slotSetMinDepth( int ) ) );
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
void RiuWellLogPlot::insertTrackPlot( RiuWellLogTrack* trackPlot, size_t index )
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
    m_legends.insert( static_cast<int>( index ), legend );

    trackPlot->updateLegend();

    if ( trackPlot->isRimTrackVisible() )
    {
        trackPlot->show();
    }
    else
    {
        trackPlot->hide();
    }

    updateChildrenLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::removeTrackPlot( RiuWellLogTrack* trackPlot )
{
    if ( !trackPlot ) return;

    int trackIdx = m_trackPlots.indexOf( trackPlot );
    CVF_ASSERT( trackIdx >= 0 );

    m_trackPlots.removeAt( trackIdx );
    trackPlot->setParent( nullptr );

    QwtLegend* legend = m_legends[trackIdx];
    m_legends.removeAt( trackIdx );
    delete legend;

    updateChildrenLayout();
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
    this->updateChildrenLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QSize RiuWellLogPlot::preferredSize() const
{
    int titleWidth  = 0;
    int titleHeight = 0;
    if ( m_plotTitle && m_plotTitle->isVisible() )
    {
        titleWidth  = m_plotTitle->width();
        titleHeight = m_plotTitle->height() + 10;
    }

    int sumTrackWidth  = 0;
    int maxTrackHeight = 0;
    for ( QPointer<RiuWellLogTrack> track : m_trackPlots )
    {
        sumTrackWidth += track->width();
        maxTrackHeight = std::max( maxTrackHeight, track->height() );
    }
    return QSize( std::max( titleWidth, sumTrackWidth ), titleHeight + maxTrackHeight );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::setTitleVisible( bool visible )
{
    m_plotTitle->setVisible( visible );
    this->updateChildrenLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::updateChildrenLayout()
{
    reinsertTracks();

    int trackCount            = m_trackPlots.size();
    int numTracksAlreadyShown = 0;
    for ( int tIdx = 0; tIdx < trackCount; ++tIdx )
    {
        if ( m_plotDefinition->areTrackLegendsVisible() && m_trackPlots[tIdx]->isVisible() )
        {
            int legendColumns = 1;
            if ( m_plotDefinition->areTrackLegendsHorizontal() )
            {
                legendColumns = 0; // unlimited
            }
            m_legends[tIdx]->setMaxColumns( legendColumns );
            m_legends[tIdx]->show();

            m_trackPlots[tIdx]->enableDepthAxisLabelsAndTitle( numTracksAlreadyShown == 0 );
            numTracksAlreadyShown++;
        }
        else
        {
            m_legends[tIdx]->hide();
        }
        RiuWellLogTrack* riuTrack = m_trackPlots[tIdx];
        m_trackLayout->setColumnStretch( tIdx, riuTrack->widthScaleFactor() );
    }
    alignCanvasTops();
    this->update();
    this->repaint();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::showEvent( QShowEvent* )
{
    updateChildrenLayout();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::changeEvent( QEvent* event )
{
    if ( event->type() == QEvent::WindowStateChange )
    {
        updateChildrenLayout();
    }
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
QSize RiuWellLogPlot::sizeHint() const
{
    return QSize( 1, 1 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QLabel* RiuWellLogPlot::createTitleLabel() const
{
    QLabel* plotTitle = new QLabel( "PLOT TITLE HERE", nullptr );
    QFont   font      = plotTitle->font();
    font.setPointSize( 14 );
    font.setBold( true );
    plotTitle->setFont( font );
    plotTitle->setVisible( m_plotDefinition->isPlotTitleVisible() );
    plotTitle->setAlignment( Qt::AlignHCenter );
    return plotTitle;
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
void RiuWellLogPlot::alignCanvasTops()
{
    CVF_ASSERT( m_legends.size() == m_trackPlots.size() );

    int maxCanvasOffset = 0;
    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        if ( m_trackPlots[tIdx]->isVisible() )
        {
            // Hack to align QWT plots. See below.
            QRectF canvasRect    = m_trackPlots[tIdx]->plotLayout()->canvasRect();
            int    canvasMargins = m_trackPlots[tIdx]->plotLayout()->canvasMargin( QwtPlot::xTop );
            maxCanvasOffset      = std::max( maxCanvasOffset, static_cast<int>( canvasRect.top() + canvasMargins ) );
        }
    }

    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        if ( m_trackPlots[tIdx]->isVisible() )
        {
            // Hack to align QWT plots which doesn't have an x-axis with the other tracks.
            // Since they are missing the axis, QWT will shift them upwards.
            // So we shift the plot downwards and resize to match the others.
            // TODO: Look into subclassing QwtPlotLayout instead.
            QRectF canvasRect     = m_trackPlots[tIdx]->plotLayout()->canvasRect();
            int    canvasMargins  = m_trackPlots[tIdx]->plotLayout()->canvasMargin( QwtPlot::xTop );
            int    myCanvasOffset = static_cast<int>( canvasRect.top() ) + canvasMargins;
            int    canvasShift    = std::max( 0, maxCanvasOffset - myCanvasOffset );

            QMargins margins = m_trackPlots[tIdx]->contentsMargins();
            margins.setTop( margins.top() + canvasShift );
            m_trackPlots[tIdx]->setContentsMargins( margins );
        }
    }

    if ( m_trackLayout->columnCount() > 0 && m_trackLayout->rowCount() > 0 )
    {
        int legendHeight = m_trackLayout->cellRect( 0, 0 ).height();
        m_scrollBarLayout->setContentsMargins( 0, legendHeight, 0, 0 );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::reinsertTracks()
{
    int visibleIndex = 0;
    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        if ( m_trackPlots[tIdx]->isVisible() )
        {
            m_trackLayout->addWidget( m_legends[tIdx], 0, static_cast<int>( visibleIndex ) );
            m_trackLayout->addWidget( m_trackPlots[tIdx], 1, static_cast<int>( visibleIndex ) );
            m_trackLayout->setRowStretch( 1, 1 );

            if ( !m_plotDefinition->areTrackLegendsVisible() )
            {
                m_legends[tIdx]->hide();
            }
            visibleIndex++;
        }
    }
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
