/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026-     Equinor ASA
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

#include "RiuGroupNetworkWidget.h"

#include "GroupNetwork/RimGroupNetworkPlot.h"

#include "RiuGroupNetworkGraphView.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QSlider>
#include <QVBoxLayout>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupNetworkWidget::RiuGroupNetworkWidget( QWidget* parent )
    : QWidget( parent )
    , m_graphView( new RiuGroupNetworkGraphView( this ) )
    , m_timeStepSlider( new QSlider( Qt::Horizontal, this ) )
    , m_dateLabel( new QLabel( this ) )
    , m_statusLabel( new QLabel( this ) )
{
    auto* timeLayout = new QHBoxLayout();
    timeLayout->addWidget( new QLabel( "Time Step", this ) );
    timeLayout->addWidget( m_timeStepSlider, 1 );
    timeLayout->addWidget( m_dateLabel );
    m_dateLabel->setMinimumWidth( 80 );

    m_statusLabel->setWordWrap( true );

    auto* layout = new QVBoxLayout( this );
    layout->setContentsMargins( 4, 4, 4, 4 );
    layout->addLayout( timeLayout );
    layout->addWidget( m_graphView, 1 );
    layout->addWidget( m_statusLabel );

    connect( m_timeStepSlider, &QSlider::valueChanged, this, &RiuGroupNetworkWidget::onTimeStepChanged );
    connect( m_graphView, &RiuGroupNetworkGraphView::nodeSelected, this, &RiuGroupNetworkWidget::nodeSelected );
    connect( m_graphView, &RiuGroupNetworkGraphView::toggleCollapseRequested, this, &RiuGroupNetworkWidget::onToggleCollapse );
    connect( m_graphView, &RiuGroupNetworkGraphView::createPlotRequested, this, &RiuGroupNetworkWidget::onCreatePlot );
    connect( m_graphView,
             &RiuGroupNetworkGraphView::expandAllRequested,
             this,
             [this]()
             {
                 if ( m_network ) m_network->expandAll();
             } );
    connect( m_graphView,
             &RiuGroupNetworkGraphView::collapseAllRequested,
             this,
             [this]()
             {
                 if ( m_network ) m_network->collapseAll();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuGroupNetworkWidget::~RiuGroupNetworkWidget()
{
    if ( m_network ) m_network->networkChanged.disconnect( this );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::setNetwork( RimGroupNetworkPlot* network )
{
    if ( network == m_network.p() ) return;

    if ( m_network )
    {
        m_network->networkChanged.disconnect( this );
    }
    m_network = network;
    if ( m_network )
    {
        m_network->networkChanged.connect( this, &RiuGroupNetworkWidget::onNetworkChanged );
    }

    m_graphView->clear();
    refresh();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimGroupNetworkPlot* RiuGroupNetworkWidget::network() const
{
    return m_network;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::clear()
{
    setNetwork( nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::zoomAll()
{
    m_graphView->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::refresh()
{
    m_refreshing = true;

    if ( !m_network )
    {
        m_graphView->clear();
        m_timeStepSlider->setRange( 0, 0 );
        m_timeStepSlider->setEnabled( false );
        m_dateLabel->clear();
        m_statusLabel->setText( "Select a production network in the project tree" );
        m_refreshing = false;
        return;
    }

    const QStringList dates = m_network->timeStepTexts();
    m_timeStepSlider->setEnabled( dates.size() > 1 );
    m_timeStepSlider->setRange( 0, std::max( 0, static_cast<int>( dates.size() ) - 1 ) );
    m_timeStepSlider->setValue( m_network->timeStepIndex() );
    m_dateLabel->setText( dates.isEmpty() ? QString() : dates[m_network->timeStepIndex()] );

    m_graphView->setNodes( m_network->graphNodes() );
    m_statusLabel->setText( m_network->statusText() );

    m_refreshing = false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::onNetworkChanged( const caf::SignalEmitter* emitter )
{
    refresh();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::onTimeStepChanged( int index )
{
    if ( m_refreshing || !m_network ) return;
    m_network->setTimeStepIndex( index );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::onToggleCollapse( const QString& nodeName )
{
    if ( !m_network ) return;
    m_network->setCollapsed( nodeName, !m_network->isCollapsed( nodeName ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuGroupNetworkWidget::onCreatePlot( const QString& nodeName )
{
    if ( !m_network ) return;
    m_network->createSummaryPlot( nodeName );
}
