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

#include "RiaFeatureCommandContext.h"

#include "RimWellPathImport.h"

#include "cafCmdFeatureMenuBuilder.h"
#include "cafPdmUiListView.h"
#include "cafPdmUiPropertyView.h"
#include "cafPdmUiTreeAttributes.h"
#include "cafPdmUiTreeView.h"
#include "cafPdmUiTreeViewEditor.h"
#include "cafUtils.h"

#include <QAbstractTableModel>
#include <QObject>
#include <QString>

#include <QtNetwork>
#include <QtWidgets>

#include <algorithm>
#include <optional>
#include <vector>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuWellImportWizard::RiuWellImportWizard( const QString&     downloadFolder,
                                          RiaOsduConnector*  osduConnector,
                                          RimWellPathImport* wellPathImportObject,
                                          QWidget*           parent /*= 0*/ )
    : QWizard( parent )
{
    m_wellPathImportObject = wellPathImportObject;

    m_osduConnector     = osduConnector;
    m_destinationFolder = downloadFolder;

    m_firstTimeRequestingAuthentication = true;

    addPage( new AuthenticationPage( m_osduConnector, this ) );
    addPage( new FieldSelectionPage( m_wellPathImportObject, m_osduConnector, this ) );
    addPage( new WellSelectionPage( m_wellPathImportObject, m_osduConnector, this ) );
    addPage( new WellSummaryPage( m_wellPathImportObject, m_osduConnector, this ) );
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
    // TODO: filter by user input
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
    m_osduConnector->requestWellsByFieldId( fieldId );
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
    m_osduConnector->requestToken();
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
FieldSelectionPage::FieldSelectionPage( RimWellPathImport* wellPathImport, RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
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

    m_tableView = new QTableView( this );
    m_tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
    QHeaderView* header = m_tableView->horizontalHeader();
    header->setSectionResizeMode( QHeaderView::Interactive );
    header->setStretchLastSection( true );

    m_osduFieldsModel = new OsduFieldTableModel;
    m_tableView->setModel( m_osduFieldsModel );
    m_tableView->setSortingEnabled( true );
    int nameColumn = 2;
    m_tableView->sortByColumn( nameColumn, Qt::AscendingOrder );

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
        int                  column  = 0;
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
WellSelectionPage::WellSelectionPage( RimWellPathImport* wellPathImport, RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
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
    m_tableView->setSelectionMode( QAbstractItemView::MultiSelection );
    m_tableView->setSortingEnabled( true );
    int nameColumn = 2;
    m_tableView->sortByColumn( nameColumn, Qt::AscendingOrder );

    QHeaderView* header = m_tableView->horizontalHeader();
    header->setSectionResizeMode( QHeaderView::Interactive );
    header->setStretchLastSection( true );

    m_osduWellboresModel = new OsduWellboreTableModel;
    layout->addWidget( m_tableView );
    layout->setStretchFactor( m_tableView, 10 );

    m_proxyModel = new QSortFilterProxyModel( this );
    m_proxyModel->setSourceModel( m_osduWellboresModel );
    m_proxyModel->setFilterKeyColumn( nameColumn );
    m_proxyModel->setFilterCaseSensitivity( Qt::CaseInsensitive );

    m_tableView->setModel( m_proxyModel );
    m_tableView->setSortingEnabled( true );

    QObject::connect( filterLineEdit, &QLineEdit::textChanged, m_proxyModel, &QSortFilterProxyModel::setFilterWildcard );

    m_wellPathImportObject = wellPathImport;

    m_osduConnector = osduConnector;
    connect( m_osduConnector, SIGNAL( wellsFinished() ), SLOT( wellsFinished() ) );
    connect( m_osduConnector, SIGNAL( wellboresFinished( const QString& ) ), SLOT( wellboresFinished( const QString& ) ) );

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
void WellSelectionPage::wellsFinished()
{
    std::vector<OsduWell> wells = m_osduConnector->wells();
    for ( auto w : wells )
    {
        m_osduConnector->requestWellboresByWellId( w.id );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::wellboresFinished( const QString& wellId )
{
    std::vector<OsduWellbore> wellbores = m_osduConnector->wellbores( wellId );
    if ( !wellbores.empty() ) m_osduWellboresModel->setOsduWellbores( wellId, wellbores );
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
            int idColumn = 0;

            if ( index.column() == idColumn )
            {
                QString wellboreId = m_proxyModel->data( index.siblingAtColumn( idColumn ) ).toString();
                wellboreIds.push_back( wellboreId );
            }
        }

        RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );
        wiz->setSelectedWellboreIds( wellboreIds );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
WellSummaryPage::WellSummaryPage( RimWellPathImport* wellPathImport, RiaOsduConnector* osduConnector, QWidget* parent /*= 0*/ )
{
    m_wellPathImportObject = wellPathImport;
    m_wellPathImportObject->setUiHidden( true );

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
    std::vector<OsduWell>               wells                = m_osduConnector->wells();

    auto findWellboreById = []( const std::vector<OsduWellbore>& wellbores, const QString& wellboreId ) -> std::optional<const OsduWellbore>
    {
        auto it = std::find_if( wellbores.begin(), wellbores.end(), [wellboreId]( const OsduWellbore& w ) { return w.id == wellboreId; } );
        if ( it != wellbores.end() )
            return std::optional<const OsduWellbore>( *it );
        else
            return {};
    };

    auto findWellForWellId = []( const std::vector<OsduWell>& wells, const QString& wellId ) -> std::optional<const OsduWell>
    {
        auto it = std::find_if( wells.begin(), wells.end(), [wellId]( const OsduWell& w ) { return w.id == wellId; } );
        if ( it != wells.end() )
            return std::optional<const OsduWell>( *it );
        else
            return {};
    };

    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );

    {
        QMutexLocker lock( &m_mutex );

        QString                       wellId = m_osduConnector->wellIdForWellboreId( wellboreId );
        std::optional<const OsduWell> well   = findWellForWellId( wells, wellId );

        std::vector<OsduWellbore>         wellbores = m_osduConnector->wellbores( wellId );
        std::optional<const OsduWellbore> wellbore  = findWellboreById( wellbores, wellboreId );

        if ( well.has_value() && wellbore.has_value() )
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
                                    .wellId               = well.value().id,
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
