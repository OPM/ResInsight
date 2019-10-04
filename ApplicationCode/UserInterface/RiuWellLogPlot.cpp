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

    m_scrollBarLayout = new QVBoxLayout;
    m_scrollBarLayout->setContentsMargins( 0, 50, 0, 0 );
    m_plotLayout->addLayout( m_scrollBarLayout );

    m_scrollBar = new QScrollBar( nullptr );
    m_scrollBar->setOrientation( Qt::Vertical );
    m_scrollBar->setVisible( true );

    m_scrollBarLayout->addWidget( m_scrollBar, 0 );

    new RiuPlotObjectPicker( m_plotTitle, m_plotDefinition );

    this->setSizePolicy( QSizePolicy::Preferred, QSizePolicy::MinimumExpanding );

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
void RiuWellLogPlot::updateChildrenLayout()
{
    reinsertTracks();
    alignCanvasTops();
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

    double maxExtent = 0.0;
    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        if ( m_trackPlots[tIdx]->isVisible() )
        {
            QFont font = m_trackPlots[tIdx]->axisFont( QwtPlot::xTop );
            maxExtent  = std::max( maxExtent, m_trackPlots[tIdx]->axisScaleDraw( QwtPlot::xTop )->extent( font ) );
        }
    }

    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        if ( m_trackPlots[tIdx]->isVisible() )
        {
            m_trackPlots[tIdx]->axisScaleDraw( QwtPlot::xTop )->setMinimumExtent( maxExtent );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::reinsertTracks()
{
    clearTrackLayout();

    int visibleIndex = 0;
    for ( int tIdx = 0; tIdx < m_trackPlots.size(); ++tIdx )
    {
        if ( m_trackPlots[tIdx]->isRimTrackVisible() )
        {
            m_trackLayout->addWidget( m_legends[tIdx], 0, static_cast<int>( visibleIndex ) );
            m_trackLayout->addWidget( m_trackPlots[tIdx], 1, static_cast<int>( visibleIndex ) );

            if ( m_plotDefinition->areTrackLegendsVisible() )
            {
                int legendColumns = 1;
                if ( m_plotDefinition->areTrackLegendsHorizontal() )
                {
                    legendColumns = 0; // unlimited
                }
                m_legends[tIdx]->setMaxColumns( legendColumns );
                m_trackPlots[tIdx]->updateLegend();
                int minimumHeight = m_legends[tIdx]->heightForWidth( m_trackPlots[tIdx]->width() );
                m_legends[tIdx]->setMinimumHeight( minimumHeight );

                m_legends[tIdx]->show();
            }
            else
            {
                m_legends[tIdx]->hide();
            }

            m_trackPlots[tIdx]->setDepthTitle( visibleIndex == 0 ? m_plotDefinition->depthPlotTitle() : "" );
            m_trackPlots[tIdx]->enableDepthAxisLabelsAndTicks( visibleIndex == 0 );
            m_trackPlots[tIdx]->show();

            int widthScaleFactor = m_trackPlots[tIdx]->widthScaleFactor();
            if ( visibleIndex == 0 )
            {
                widthScaleFactor += 1; // Give it a bit extra room due to depth axis
            }
            m_trackLayout->setColumnStretch( visibleIndex, widthScaleFactor );
            m_trackLayout->setRowStretch( 1, 1 );
            visibleIndex++;
        }
        else
        {
            m_trackPlots[tIdx]->hide();
            m_legends[tIdx]->hide();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogPlot::clearTrackLayout()
{
    if ( m_trackLayout )
    {
        QLayoutItem* item;
        while ( ( item = m_trackLayout->takeAt( 0 ) ) != 0 )
        {
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
