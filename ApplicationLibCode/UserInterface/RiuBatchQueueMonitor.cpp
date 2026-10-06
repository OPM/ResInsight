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

#include "RiuBatchMonitorWorker.h"
#include "RiuMessagePanel.h"
#include "RiuPlotMainWindow.h"

#include <QCheckBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QPushButton>
#include <QTableWidget>
#include <QThread>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuBatchQueueMonitor::RiuBatchQueueMonitor( QWidget* pParent )
    : QWidget( pParent )
{
    m_checkBoxAutoUpdate = new QCheckBox( "Enable Auto Update", this );
    m_checkBoxAutoUpdate->setChecked( false );

    QHBoxLayout* pTopLayout = new QHBoxLayout;
    pTopLayout->addWidget( m_checkBoxAutoUpdate );
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
void RiuBatchQueueMonitor::slotUpdateView( const QStringList& information )
{
    m_jobView->setRowCount( 0 );
    for ( auto& line : information )
    {
        int newRowIndex = m_jobView->rowCount();
        m_jobView->insertRow( newRowIndex );

        auto parts = line.split( ' ', Qt::SplitBehaviorFlags::SkipEmptyParts );

        for ( int i = 0; i < parts.size(); ++i )
        {
            m_jobView->setItem( newRowIndex, i, new QTableWidgetItem( parts.at( i ) ) );
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
        m_monitorWorker = new RiuBatchMonitorWorker();
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
        if ( m_monitorWorker != nullptr )
        {
            m_monitorWorker->stopMonitoring();
            m_monitorWorker = nullptr;
        }
    }
}
