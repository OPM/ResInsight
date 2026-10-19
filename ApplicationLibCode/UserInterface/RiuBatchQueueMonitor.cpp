/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2026 Equinor ASA
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

#include "RiuBatchQueueMonitor.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferencesBatch.h"

#include "RiuBatchMonitorWorker.h"
#include "RiuMessagePanel.h"
#include "RiuPlotMainWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QTableWidget>
#include <QThread>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuBatchQueueMonitor::RiuBatchQueueMonitor( QWidget* pParent )
    : QWidget( pParent )
    , m_monitorWorker( nullptr )
{
    m_checkBoxAutoUpdate = new QCheckBox( "Enable Job Monitor", this );
    m_checkBoxAutoUpdate->setChecked( false );

    m_onlyResInsightJobs = new QCheckBox( "Only Show ResInsight Jobs", this );
    m_onlyResInsightJobs->setChecked( true );

    QLabel* label = new QLabel( "Refresh Interval:", this );

    m_refreshInterval = new QComboBox( this );
    m_refreshInterval->addItem( "5 seconds", QVariant( 5 ) );
    m_refreshInterval->addItem( "10 seconds", QVariant( 10 ) );
    m_refreshInterval->addItem( "30 seconds", QVariant( 30 ) );
    m_refreshInterval->setCurrentIndex( 1 );
    m_refreshInterval->setEditable( false );

    QHBoxLayout* pTopLayout = new QHBoxLayout;
    pTopLayout->addWidget( m_checkBoxAutoUpdate );
    pTopLayout->addSpacing( 5 );
    pTopLayout->addWidget( m_onlyResInsightJobs );
    pTopLayout->addSpacing( 10 );
    pTopLayout->addWidget( label );
    pTopLayout->addWidget( m_refreshInterval );
    pTopLayout->addStretch();

    m_jobView = new QTableWidget( this );
    m_jobView->setColumnCount( 6 );
    m_jobView->setRowCount( 1 );
    m_jobView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_jobView->setSizeAdjustPolicy( QAbstractScrollArea::AdjustToContents );
    m_jobView->verticalHeader()->setVisible( false );

    QStringList headerNames;
    headerNames << "Job ID" << "Job Name" << "Run Time" << "State" << "Queue/Partition" << "Where";

    m_jobView->setHorizontalHeaderLabels( headerNames );

    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addLayout( pTopLayout );
    pLayout->addWidget( m_jobView, 1 );

    pLayout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( pLayout );

    connect( m_checkBoxAutoUpdate, SIGNAL( clicked() ), this, SLOT( toggleUpdates() ) );
    connect( m_refreshInterval, SIGNAL( currentIndexChanged( int ) ), this, SLOT( updateInterval() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuBatchQueueMonitor::~RiuBatchQueueMonitor()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchQueueMonitor::slotUpdateView( const std::vector<std::vector<QString>> information )
{
    m_jobView->setRowCount( 0 );
    for ( auto& line : information )
    {
        int newRowIndex = m_jobView->rowCount();
        m_jobView->insertRow( newRowIndex );

        if ( line.size() < 2 ) continue;

        // only show jobs with ResInsight prefix?
        if ( m_onlyResInsightJobs->isChecked() && !line[1].startsWith( "RI_" ) ) continue;

        for ( int i = 0; i < (int)line.size(); i++ )
        {
            auto item = new QTableWidgetItem( line[i] );
            if ( i == 2 ) // time should be right-aligned
            {
                item->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );
            }
            m_jobView->setItem( newRowIndex, i, item );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchQueueMonitor::toggleUpdates()
{
    if ( m_checkBoxAutoUpdate->isChecked() )
    {
        auto schedulerType = RiaPreferencesBatch::current()->batchScheduler();

        int interval = m_refreshInterval->currentData().toInt();
        if ( interval < 5 ) interval = 5; // minimum interval of 5 seconds
        if ( interval > 60 ) interval = 60; // maximum interval of 60 seconds

        m_monitorWorker = new RiuBatchMonitorWorker( schedulerType, interval );
        auto thread     = new QThread( this );

        m_monitorWorker->moveToThread( thread );

        connect( thread, &QThread::started, m_monitorWorker, &RiuBatchMonitorWorker::gatherInformation );
        connect( thread, &QThread::finished, m_monitorWorker, &QObject::deleteLater );
        connect( thread, &QThread::finished, thread, &QObject::deleteLater );
        connect( m_monitorWorker, &RiuBatchMonitorWorker::informationGathered, this, &RiuBatchQueueMonitor::slotUpdateView );
        connect( m_monitorWorker, &RiuBatchMonitorWorker::finished, thread, &QThread::quit );

        thread->start();
    }
    else
    {
        stopMonitoring();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchQueueMonitor::stopMonitoring()
{
    if ( m_monitorWorker != nullptr )
    {
        m_monitorWorker->stopMonitoring();
        m_monitorWorker = nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchQueueMonitor::updateInterval()
{
    stopMonitoring();

    if ( m_checkBoxAutoUpdate->isChecked() )
    {
        toggleUpdates();
    }
}
