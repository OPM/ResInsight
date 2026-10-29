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

#include "RiuBatchJobMonitorView.h"

#include "Hpc/RiaBatchMonitorService.h"
#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferencesBatch.h"
#include "RiaPreferencesOpm.h"

#include "RiuMessagePanel.h"
#include "RiuPlotMainWindow.h"

#include <QCheckBox>
#include <QComboBox>
#include <QHBoxLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMenu>
#include <QPushButton>
#include <QTableWidget>
#include <QThread>
#include <QWidget>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuBatchJobMonitorView::RiuBatchJobMonitorView( QWidget* pParent )
    : QWidget( pParent )
{
    m_checkBoxAutoUpdate = new QCheckBox( "Enable Job Monitor", this );
    m_checkBoxAutoUpdate->setChecked( false );

    m_onlyResInsightJobs = new QCheckBox( "Only Show ResInsight Jobs", this );
    m_onlyResInsightJobs->setChecked( true );

    QHBoxLayout* pTopLayout = new QHBoxLayout;
    pTopLayout->addWidget( m_checkBoxAutoUpdate );
    pTopLayout->addSpacing( 10 );
    pTopLayout->addWidget( m_onlyResInsightJobs );
    pTopLayout->addStretch();

    m_jobView = new QTableWidget( this );
    m_jobView->setColumnCount( 6 );
    m_jobView->setRowCount( 1 );
    m_jobView->setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding );
    m_jobView->setSizeAdjustPolicy( QAbstractScrollArea::AdjustToContents );
    m_jobView->verticalHeader()->setVisible( false );
    m_jobView->setContextMenuPolicy( Qt::CustomContextMenu );

    m_infoLabel = new QLabel( this );
    m_infoLabel->setText( " Jobs are set to run on local computer only, batch job monitor is disabled." );

    QStringList headerNames;
    headerNames << "Job ID" << "Job Name" << "Run Time" << "State" << "Queue/Partition" << "Where";

    m_jobView->setHorizontalHeaderLabels( headerNames );
    m_jobView->setRowCount( 0 );

    QVBoxLayout* pLayout = new QVBoxLayout();
    pLayout->addLayout( pTopLayout );
    pLayout->addWidget( m_jobView, 1 );
    pLayout->addWidget( m_infoLabel );

    pLayout->setContentsMargins( 0, 0, 0, 0 );

    setLayout( pLayout );

    connect( m_checkBoxAutoUpdate, SIGNAL( clicked() ), this, SLOT( toggleUpdates() ) );
    connect( RiaBatchMonitorService::instance(),
             SIGNAL( jobInfoUpdate( const std::map<QString, RiaHpcTools::BatchJobInformation> ) ),
             this,
             SLOT( slotUpdateView( const std::map<QString, RiaHpcTools::BatchJobInformation> ) ) );

    connect( m_jobView, &QTableView::customContextMenuRequested, this, &RiuBatchJobMonitorView::showContextMenu );

    rebuildGui();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuBatchJobMonitorView::~RiuBatchJobMonitorView()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchJobMonitorView::rebuildGui()
{
    auto prefs = RiaPreferencesBatch::current();

    if ( prefs->batchScheduler() == RiaDefines::BatchSchedulerType::LOCAL_COMPUTER )
    {
        m_checkBoxAutoUpdate->hide();
        m_onlyResInsightJobs->hide();
        m_infoLabel->show();
    }
    else
    {
        m_checkBoxAutoUpdate->show();
        m_onlyResInsightJobs->show();
        m_infoLabel->hide();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchJobMonitorView::slotUpdateView( const std::map<QString, RiaHpcTools::BatchJobInformation> information )
{
    if ( !m_checkBoxAutoUpdate->isChecked() ) return;

    m_jobView->setRowCount( 0 );
    for ( auto& [key, value] : information )
    {
        auto line = value.asStringList();
        if ( line.size() < 2 ) continue;

        int newRowIndex = m_jobView->rowCount();
        m_jobView->insertRow( newRowIndex );

        // only show jobs with ResInsight prefix?
        if ( m_onlyResInsightJobs->isChecked() && !line[1].startsWith( "RI_" ) ) continue;

        for ( int i = 0; i < (int)line.size(); i++ )
        {
            auto item = new QTableWidgetItem( line[i] );
            if ( i == 2 ) // time should be right-aligned
            {
                item->setTextAlignment( Qt::AlignRight | Qt::AlignVCenter );
            }
            item->setFlags( item->flags() & ~Qt::ItemIsEditable );
            m_jobView->setItem( newRowIndex, i, item );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchJobMonitorView::toggleUpdates()
{
    if ( m_checkBoxAutoUpdate->isChecked() )
    {
        RiaBatchMonitorService::start();
    }
    else
    {
        RiaBatchMonitorService::stop();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchJobMonitorView::showEvent( QShowEvent* event )
{
    if ( m_checkBoxAutoUpdate->isChecked() )
    {
        RiaBatchMonitorService::start();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchJobMonitorView::hideEvent( QHideEvent* event )
{
    if ( m_checkBoxAutoUpdate->isChecked() )
    {
        RiaBatchMonitorService::stop();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchJobMonitorView::showContextMenu( const QPoint& pos )
{
    QModelIndex index = m_jobView->indexAt( pos );
    if ( !index.isValid() ) return;
    if ( index.column() != 0 ) return;

    QPoint   globalPos = m_jobView->viewport()->mapToGlobal( pos );
    QMenu    menu;
    QAction* stopAction = menu.addAction( "Stop Job" );
    connect( stopAction,
             &QAction::triggered,
             this,
             [this, index]
             {
                 QString jobId = m_jobView->model()->data( index ).toString();
                 stopJob( jobId );
             } );

    menu.exec( globalPos );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuBatchJobMonitorView::stopJob( QString jobId )
{
    if ( jobId.trimmed().isEmpty() ) return;
    auto prefs    = RiaPreferencesBatch::current();
    auto opmPrefs = RiaPreferencesOpm::current();

    RiaHpcTools::stopJob( prefs->batchScheduler(), jobId, opmPrefs->wslOptions() );
}
