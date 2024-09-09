/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024-     Equinor ASA
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

#include "RiuWellLogImportWizard.h"

#include "RiaFeatureCommandContext.h"

#include <QAbstractTableModel>
#include <QObject>
#include <QString>
#include <QTextEdit>

#include <QtNetwork>
#include <QtWidgets>

#include <algorithm>
#include <optional>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogImportWizard::RiuWellLogImportWizard( RiaOsduConnector* osduConnector, const QString& wellboreId, QWidget* parent /*= 0*/ )
    : QWizard( parent )
{
    m_osduConnector = osduConnector;
    m_wellboreId    = wellboreId;

    addPage( new WellLogAuthenticationPage( m_osduConnector, this ) );
    addPage( new WellLogSelectionPage( m_osduConnector, this ) );

    setMinimumSize( 800, 800 );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellLogImportWizard::~RiuWellLogImportWizard()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogImportWizard::downloadWellLogs( const QString& wellboreId )
{
    m_osduConnector->requestWellLogsByWellboreId( wellboreId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogImportWizard::setSelectedWellLogs( const std::vector<QString>& wellboreIds )
{
    m_selectedWellLogs = wellboreIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiuWellLogImportWizard::selectedWellLogs() const
{
    return m_selectedWellLogs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellLogImportWizard::wellboreId() const
{
    return m_wellboreId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<OsduWellLog> RiuWellLogImportWizard::importedWellLogs() const
{
    return m_wellLogs;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellLogImportWizard::addWellLog( OsduWellLog wellLog )
{
    m_wellLogs.push_back( wellLog );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellLogAuthenticationPage::WellLogAuthenticationPage( RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
    : QWizardPage( parent )
    , m_osduConnector( osduConnector )
    , m_accessOk( false )
{
    setTitle( "OSDU - Login" );

    QVBoxLayout* layout = new QVBoxLayout;

    m_connectionLabel = new QLabel( "Checking OSDU connection. You might need to login." );
    layout->addWidget( m_connectionLabel );

    QFormLayout* formLayout = new QFormLayout;
    layout->addLayout( formLayout );

    QLineEdit* serverLineEdit = new QLineEdit( osduConnector->server(), this );
    serverLineEdit->setReadOnly( true );
    QLineEdit* partitionLineEdit = new QLineEdit( osduConnector->dataPartition(), this );
    partitionLineEdit->setReadOnly( true );

    formLayout->addRow( "Server:", serverLineEdit );
    formLayout->addRow( "Data Partition:", partitionLineEdit );

    setLayout( layout );

    connect( osduConnector, SIGNAL( tokenReady( const QString& ) ), this, SLOT( accessOk() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellLogAuthenticationPage::initializePage()
{
    m_osduConnector->requestToken();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WellLogAuthenticationPage::isComplete() const
{
    return m_accessOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellLogAuthenticationPage::accessOk()
{
    m_connectionLabel->setText( "Connection to OSDU: OK." );
    m_accessOk = true;
    emit( completeChanged() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellLogSelectionPage::WellLogSelectionPage( RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    QLabel* label = new QLabel( "Select well logs" );
    layout->addWidget( label );

    QHBoxLayout* filterLayout = new QHBoxLayout;
    filterLayout->addWidget( new QLabel( "Filter:", this ) );
    QLineEdit* filterLineEdit = new QLineEdit( this );
    filterLayout->addWidget( filterLineEdit );

    layout->addLayout( filterLayout );

    m_tableView = new QTableView( this );
    m_tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_tableView->setSelectionMode( QAbstractItemView::ExtendedSelection );
    m_tableView->setSortingEnabled( true );
    int nameColumn = 2;
    m_tableView->sortByColumn( nameColumn, Qt::AscendingOrder );

    QHeaderView* header = m_tableView->horizontalHeader();
    header->setSectionResizeMode( QHeaderView::Interactive );

    m_osduWellLogsModel = new OsduWellLogTableModel;
    layout->addWidget( m_tableView );
    layout->setStretchFactor( m_tableView, 10 );

    m_proxyModel = new QSortFilterProxyModel( this );
    m_proxyModel->setSourceModel( m_osduWellLogsModel );
    m_proxyModel->setFilterKeyColumn( nameColumn );
    m_proxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

    m_tableView->setModel( m_proxyModel );
    m_tableView->setSortingEnabled( true );

    // Hide the ID and kind columns
    m_tableView->hideColumn( 0 );
    m_tableView->hideColumn( 1 );

    m_detailText = new QTextEdit( this );
    m_detailText->setReadOnly( true );
    layout->addWidget( m_detailText );

    QObject::connect( filterLineEdit, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterWildcard );

    m_osduConnector = osduConnector;
    connect( m_osduConnector, SIGNAL( wellLogsFinished( const QString& ) ), SLOT( wellLogsFinished( const QString& ) ) );

    connect( m_tableView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SLOT( selectWellLogs( const QItemSelection&, const QItemSelection& ) ) );

    connect( m_tableView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SIGNAL( completeChanged() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellLogSelectionPage::initializePage()
{
    RiuWellLogImportWizard* wiz = dynamic_cast<RiuWellLogImportWizard*>( wizard() );
    if ( !wiz ) return;

    QString wellboreId = wiz->wellboreId();
    wiz->downloadWellLogs( wellboreId );

    setButtonText( QWizard::NextButton, "Next" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellLogSelectionPage::~WellLogSelectionPage()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellLogSelectionPage::wellLogsFinished( const QString& wellboreId )
{
    std::vector<OsduWellLog> wellLogs = m_osduConnector->wellLogs( wellboreId );
    m_osduWellLogsModel->setOsduWellLogs( wellLogs );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WellLogSelectionPage::isComplete() const
{
    QItemSelectionModel* select = m_tableView->selectionModel();
    return !select->selectedRows().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellLogSelectionPage::selectWellLogs( const QItemSelection& newSelection, const QItemSelection& oldSelection )
{
    if ( !newSelection.indexes().empty() )
    {
        RiuWellLogImportWizard* wiz = dynamic_cast<RiuWellLogImportWizard*>( wizard() );

        std::vector<OsduWellLog> wellLogs = m_osduConnector->wellLogs( wiz->wellboreId() );

        auto findWellLogById = []( const std::vector<OsduWellLog>& wellLogs, const QString& wellLogId ) -> std::optional<const OsduWellLog>
        {
            auto it = std::find_if( wellLogs.begin(), wellLogs.end(), [wellLogId]( const OsduWellLog& w ) { return w.id == wellLogId; } );
            if ( it != wellLogs.end() )
                return std::optional<const OsduWellLog>( *it );
            else
                return {};
        };

        auto generateWellLogDetailsText = []( const OsduWellLog& wellLog ) -> QString
        {
            QString text = QString( "Name: %1\n" ).arg( wellLog.name );
            if ( !wellLog.description.isEmpty() )
            {
                text.append( QString( "Description: %2\n" ).arg( wellLog.description ) );
            }

            for ( auto channel : wellLog.channels )
            {
                QString channelText = QString( "  %1: \"%2\". Depth: %3 - %4." )
                                          .arg( channel.mnemonic )
                                          .arg( channel.description )
                                          .arg( channel.topDepth )
                                          .arg( channel.baseDepth );
                if ( !channel.interpreterName.isEmpty() )
                {
                    channelText.append( QString( " Interpreter: %1." ).arg( channel.interpreterName ) );
                }

                if ( !channel.quality.isEmpty() )
                {
                    channelText.append( QString( " Quality: %1." ).arg( channel.quality ) );
                }

                text.append( channelText + "\n" );
            }

            return text;
        };

        QModelIndexList selection = m_tableView->selectionModel()->selectedRows();
        for ( QModelIndex index : selection )
        {
            int idColumn = 0;

            if ( index.column() == idColumn )
            {
                QString                          wellLogId = m_proxyModel->data( index.siblingAtColumn( idColumn ) ).toString();
                std::optional<const OsduWellLog> wellLog   = findWellLogById( wellLogs, wellLogId );
                if ( wellLog.has_value() )
                {
                    wiz->addWellLog( wellLog.value() );
                    m_detailText->setText( generateWellLogDetailsText( wellLog.value() ) );
                }
            }
        }
    }
}
