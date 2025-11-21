/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiuWellImportWizard.h"

#include <QAbstractTableModel>
#include <QObject>
#include <QString>

#include <QtNetwork>
#include <QtWidgets>

#include <optional>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellImportWizard::RiuWellImportWizard( RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
    : QWizard( parent )
{
    m_osduConnector = osduConnector;

    m_firstTimeRequestingAuthentication = true;

    addPage( new AuthenticationPage( m_osduConnector, this ) );
    addPage( new FieldSelectionPage( m_osduConnector, this ) );
    addPage( new WellSelectionPage( m_osduConnector, this ) );
    addPage( new WellSummaryPage( m_osduConnector, this ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellImportWizard::~RiuWellImportWizard()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadFields( const QString& fieldName )
{
    m_osduConnector->requestFieldsByName( fieldName );
}

//--------------------------------------------------------------------------------------------------
/// This slot will be called for the first network reply that will need authentication.
/// If the authentication is successful, the username/password is cached in the QNetworkAccessManager
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::slotAuthenticationRequired( QNetworkReply* networkReply, QAuthenticator* authenticator )
{
    if ( m_firstTimeRequestingAuthentication )
    {
        m_firstTimeRequestingAuthentication = false;
    }
    else
    {
        QMessageBox::information( this,
                                  "Authentication failed",
                                  "Failed to authenticate. You will now be directed back to the first wizard page." );
        m_firstTimeRequestingAuthentication = true;

        restart();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadWells( const QString& fieldId )
{
    m_osduConnector->clearCachedData();
    m_osduConnector->requestWellboresByFieldId( fieldId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::downloadWellPaths( const QString& wellboreId )
{
    m_osduConnector->requestWellboreTrajectoryByWellboreId( wellboreId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::resetAuthenticationCount()
{
    m_firstTimeRequestingAuthentication = true;

    if ( !m_osduConnector->isGranted() )
    {
        m_osduConnector->requestTokenWithCancelButton();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setSelectedFieldId( const QString& fieldId )
{
    m_selectedFieldId = fieldId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellImportWizard::selectedFieldId() const
{
    return m_selectedFieldId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::setSelectedWellboreIds( const std::vector<QString>& wellboreIds )
{
    m_selectedWellboreIds = wellboreIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QString> RiuWellImportWizard::selectedWellboreIds() const
{
    return m_selectedWellboreIds;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiuWellImportWizard::WellInfo> RiuWellImportWizard::importedWells() const
{
    return m_wellInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiuWellImportWizard::addWellInfo( RiuWellImportWizard::WellInfo wellInfo )
{
    m_wellInfos.push_back( wellInfo );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
AuthenticationPage::AuthenticationPage( RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
    : QWizardPage( parent )
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

    if ( osduConnector->isGranted() ) accessOk();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AuthenticationPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );
    wiz->resetAuthenticationCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool AuthenticationPage::isComplete() const
{
    return m_accessOk;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void AuthenticationPage::accessOk()
{
    m_connectionLabel->setText( "Connection to OSDU: OK." );
    m_accessOk = true;
    emit( completeChanged() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldSelectionPage::FieldSelectionPage( RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
{
    setTitle( "Field Selection" );

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    QHBoxLayout* searchLayout = new QHBoxLayout;
    m_searchTextEdit          = new QLineEdit( this );
    searchLayout->addWidget( m_searchTextEdit );

    m_searchButton = new QPushButton( "Search", this );
    m_searchButton->setEnabled( false );
    searchLayout->addWidget( m_searchButton );

    layout->addLayout( searchLayout );

    QLabel* label = new QLabel( "Select fields" );
    layout->addWidget( label );

    int nameColumn = OsduFieldTableModel::Column::Name;

    m_tableView = new QTableView( this );
    m_tableView->setSelectionBehavior( QAbstractItemView::SelectRows );

    m_osduFieldsModel = new OsduFieldTableModel;
    m_tableView->setModel( m_osduFieldsModel );
    m_tableView->setSortingEnabled( true );
    m_tableView->sortByColumn( nameColumn, Qt::AscendingOrder );

    QHeaderView* header = m_tableView->horizontalHeader();
    header->setSectionResizeMode( QHeaderView::Interactive );
    header->setSectionResizeMode( nameColumn, QHeaderView::ResizeToContents );
    header->setStretchLastSection( true );

    layout->addWidget( m_tableView );
    layout->setStretchFactor( m_tableView, 10 );

    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    m_osduConnector = osduConnector;
    connect( m_osduConnector, SIGNAL( fieldsFinished() ), SLOT( fieldsFinished() ) );

    connect( m_searchTextEdit, SIGNAL( textChanged( const QString& ) ), this, SLOT( onSearchTextChanged( const QString& ) ) );

    connect( m_searchButton, SIGNAL( clicked() ), this, SLOT( searchForFields() ) );

    connect( m_tableView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SLOT( selectField( const QItemSelection&, const QItemSelection& ) ) );

    connect( m_tableView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SIGNAL( completeChanged() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldSelectionPage::onSearchTextChanged( const QString& text )
{
    m_searchButton->setEnabled( text.length() >= MINIMUM_CHARACTERS_FOR_SEARCH );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldSelectionPage::searchForFields()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );

    QString text = m_searchTextEdit->text();
    if ( text.length() >= MINIMUM_CHARACTERS_FOR_SEARCH ) wiz->downloadFields( text );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldSelectionPage::fieldsFinished()
{
    std::vector<OsduField> fields = m_osduConnector->fields();
    m_osduFieldsModel->setOsduFields( fields );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void FieldSelectionPage::selectField( const QItemSelection& newSelection, const QItemSelection& oldSelection )
{
    if ( !newSelection.indexes().empty() )
    {
        QModelIndex          index   = newSelection.indexes()[0];
        int                  column  = OsduFieldTableModel::Column::Id;
        QString              fieldId = m_osduFieldsModel->data( index.siblingAtColumn( column ) ).toString();
        RiuWellImportWizard* wiz     = dynamic_cast<RiuWellImportWizard*>( wizard() );
        wiz->setSelectedFieldId( fieldId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool FieldSelectionPage::isComplete() const
{
    QItemSelectionModel* select = m_tableView->selectionModel();
    return select->selectedRows().size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
FieldSelectionPage::~FieldSelectionPage()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellSelectionPage::WellSelectionPage( RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
{
    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    QLabel* label = new QLabel( "Select wells" );
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
    int nameColumn = OsduWellboreTableModel::Column::Name;
    m_tableView->sortByColumn( nameColumn, Qt::AscendingOrder );

    m_osduWellboresModel = new OsduWellboreTableModel;
    layout->addWidget( m_tableView );
    layout->setStretchFactor( m_tableView, 10 );

    m_proxyModel = new QSortFilterProxyModel( this );
    m_proxyModel->setSourceModel( m_osduWellboresModel );
    m_proxyModel->setFilterKeyColumn( nameColumn );
    m_proxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

    m_tableView->setModel( m_proxyModel );
    m_tableView->setSortingEnabled( true );

    QHeaderView* header = m_tableView->horizontalHeader();
    header->setSectionResizeMode( QHeaderView::Interactive );
    header->setSectionResizeMode( nameColumn, QHeaderView::ResizeToContents );
    header->setStretchLastSection( true );

    QObject::connect( filterLineEdit, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterWildcard );

    m_osduConnector = osduConnector;
    connect( m_osduConnector, SIGNAL( wellboresByFieldIdFinished( const QString& ) ), SLOT( wellboresByFieldIdFinished( const QString& ) ) );

    connect( m_tableView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SLOT( selectWellbore( const QItemSelection&, const QItemSelection& ) ) );

    connect( m_tableView->selectionModel(),
             SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ),
             SIGNAL( completeChanged() ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );
    if ( !wiz ) return;

    QString fieldId = wiz->selectedFieldId();
    wiz->downloadWells( fieldId );

    setButtonText( QWizard::NextButton, "Next" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellSelectionPage::~WellSelectionPage()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::wellboresByFieldIdFinished( const QString& fieldId )
{
    RiuWellImportWizard* wiz             = dynamic_cast<RiuWellImportWizard*>( wizard() );
    QString              selectedFieldId = wiz->selectedFieldId();

    std::vector<OsduWellbore> wellbores = m_osduConnector->wellboresByFieldId( selectedFieldId );

    // Extract unique well IDs from wellbores
    std::set<QString> uniqueWellIds;
    for ( const auto& wellbore : wellbores )
    {
        if ( !wellbore.wellId.isEmpty() )
        {
            uniqueWellIds.insert( wellbore.wellId );
        }
    }

    // Request wells for each unique well ID
    for ( const QString& wellId : uniqueWellIds )
    {
        // For now, we just populate the wellbores directly since we already have them
        // The wellId is already in the wellbore structure
        std::vector<OsduWellbore> wellboresForWell;
        for ( const auto& wellbore : wellbores )
        {
            if ( wellbore.wellId == wellId )
            {
                wellboresForWell.push_back( wellbore );
            }
        }
        m_osduWellboresModel->setOsduWellbores( wellId, wellboresForWell );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WellSelectionPage::isComplete() const
{
    QItemSelectionModel* select = m_tableView->selectionModel();
    return !select->selectedRows().empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::selectWellbore( const QItemSelection& newSelection, const QItemSelection& oldSelection )
{
    if ( !newSelection.indexes().empty() )
    {
        std::vector<QString> wellboreIds;
        QModelIndexList      selection = m_tableView->selectionModel()->selectedRows();
        for ( QModelIndex index : selection )
        {
            int     idColumn   = OsduWellboreTableModel::Column::Id;
            QString wellboreId = m_proxyModel->data( index.siblingAtColumn( idColumn ) ).toString();
            wellboreIds.push_back( wellboreId );
        }

        RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );
        wiz->setSelectedWellboreIds( wellboreIds );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellSummaryPage::WellSummaryPage( RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
{
    m_osduConnector = osduConnector;

    QVBoxLayout* layout = new QVBoxLayout;
    setLayout( layout );

    m_textEdit = new QTextEdit( this );
    m_textEdit->setReadOnly( true );
    layout->addWidget( m_textEdit );

    setButtonText( QWizard::FinishButton, "Import" );

    connect( m_osduConnector,
             SIGNAL( wellboreTrajectoryFinished( const QString&, int, const QString& ) ),
             SLOT( wellboreTrajectoryFinished( const QString&, int, const QString& ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSummaryPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );

    QMutexLocker lock( &m_mutex );
    m_pendingWellboreIds.clear();
    for ( const QString& wellboreId : wiz->selectedWellboreIds() )
    {
        m_pendingWellboreIds.insert( wellboreId );
    }

    for ( const QString& wellboreId : wiz->selectedWellboreIds() )
    {
        wiz->downloadWellPaths( wellboreId );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSummaryPage::wellboreTrajectoryFinished( const QString& wellboreId, int numTrajectories, const QString& errorMessage )
{
    std::vector<OsduWellboreTrajectory> wellboreTrajectories = m_osduConnector->wellboreTrajectories( wellboreId );

    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );

    {
        QMutexLocker lock( &m_mutex );

        std::optional<OsduWellbore> wellbore = m_osduConnector->wellboreById( wellboreId );
        if ( wellbore.has_value() )
        {
            if ( !errorMessage.isEmpty() )
            {
                m_textEdit->append( QString( "Wellbore '%1' download failed: %2." ).arg( wellbore.value().name ).arg( errorMessage ) );
            }
            else if ( numTrajectories == 0 )
            {
                m_textEdit->append( QString( "Wellbore '%1': No trajectory found." ).arg( wellbore.value().name ) );
            }

            for ( auto w : wellboreTrajectories )
            {
                QString wellboreTrajectoryId = w.id;
                wiz->addWellInfo( { .name                 = wellbore.value().name,
                                    .wellId               = wellbore.value().wellId,
                                    .wellboreId           = w.wellboreId,
                                    .wellboreTrajectoryId = wellboreTrajectoryId,
                                    .datumElevation       = wellbore.value().datumElevation } );
            }
        }

        m_pendingWellboreIds.erase( wellboreId );
    }

    emit( completeChanged() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool WellSummaryPage::isComplete() const
{
    QMutexLocker lock( &m_mutex );
    return m_pendingWellboreIds.empty();
}
