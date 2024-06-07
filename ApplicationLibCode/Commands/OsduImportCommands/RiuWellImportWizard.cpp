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
#include "RiaOsduConnector.h"

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
void RiuWellImportWizard::downloadFields()
{
    // TODO: filter by user input
    m_osduConnector->requestFieldsByName( "CASTBERG" );
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
void RiuWellImportWizard::setSelectedWellboreId( const QString& wellboreId )
{
    m_selectedWellboreId = wellboreId;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiuWellImportWizard::selectedWellboreId() const
{
    return m_selectedWellboreId;
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

    QLabel* label = new QLabel( "Checking OSDU connection..." );
    layout->addWidget( label );

    QFormLayout* formLayout = new QFormLayout;
    layout->addLayout( formLayout );

    QLineEdit* serverLineEdit    = new QLineEdit( osduConnector->server(), this );
    QLineEdit* partitionLineEdit = new QLineEdit( osduConnector->dataPartition(), this );

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

    QLabel* label = new QLabel( "Select fields" );
    layout->addWidget( label );

    m_tableView = new QTableView( this );
    m_tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_osduFieldsModel = new OsduFieldTableModel;
    m_tableView->setModel( m_osduFieldsModel );
    layout->addWidget( m_tableView );
    layout->setStretchFactor( m_tableView, 10 );

    // Tree view
    // caf::PdmUiTreeView* treeView = new caf::PdmUiTreeView( this );
    // treeView->setPdmItem( wellPathImport );
    // layout->addWidget( treeView );
    // layout->setStretchFactor( treeView, 10 );

    setSizePolicy( QSizePolicy( QSizePolicy::Expanding, QSizePolicy::Expanding ) );

    m_osduConnector = osduConnector;
    connect( m_osduConnector, SIGNAL( fieldsFinished() ), SLOT( fieldsFinished() ) );

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
void FieldSelectionPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );
    wiz->downloadFields();
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

    m_tableView = new QTableView( this );
    m_tableView->setSelectionBehavior( QAbstractItemView::SelectRows );
    m_osduWellboresModel = new OsduWellboreTableModel;
    m_tableView->setModel( m_osduWellboresModel );
    layout->addWidget( m_tableView );
    layout->setStretchFactor( m_tableView, 10 );

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
    return select->selectedRows().size() == 1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSelectionPage::selectWellbore( const QItemSelection& newSelection, const QItemSelection& oldSelection )
{
    if ( !newSelection.indexes().empty() )
    {
        QModelIndex          index      = newSelection.indexes()[0];
        int                  column     = 0;
        QString              wellboreId = m_osduWellboresModel->data( index.siblingAtColumn( column ) ).toString();
        RiuWellImportWizard* wiz        = dynamic_cast<RiuWellImportWizard*>( wizard() );
        wiz->setSelectedWellboreId( wellboreId );
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

    connect( m_osduConnector, SIGNAL( wellboreTrajectoryFinished( const QString& ) ), SLOT( wellboreTrajectoryFinished( const QString& ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSummaryPage::initializePage()
{
    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );

    QString wellboreId = wiz->selectedWellboreId();
    wiz->downloadWellPaths( wellboreId );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void WellSummaryPage::wellboreTrajectoryFinished( const QString& wellboreId )
{
    std::vector<OsduWellboreTrajectory> wellboreTrajectories = m_osduConnector->wellboreTrajectories( wellboreId );
    std::vector<OsduWell>               wells                = m_osduConnector->wells();

    auto findWellForWellId = []( const std::vector<OsduWell>& wells, const QString& wellId ) -> std::optional<const OsduWell>
    {
        auto it = std::find_if( wells.begin(), wells.end(), [wellId]( const OsduWell& w ) { return w.id == wellId; } );
        if ( it != wells.end() )
            return std::optional<const OsduWell>( *it );
        else
            return {};
    };

    RiuWellImportWizard* wiz = dynamic_cast<RiuWellImportWizard*>( wizard() );

    for ( auto w : wellboreTrajectories )
    {
        QString                       wellId = m_osduConnector->wellIdForWellboreId( w.wellboreId );
        std::optional<const OsduWell> well   = findWellForWellId( wells, wellId );

        if ( well.has_value() )
        {
            QString wellboreTrajectoryId = w.id;
            wiz->addWellInfo( { .name                 = well.value().name,
                                .wellId               = well.value().id,
                                .wellboreId           = w.wellboreId,
                                .wellboreTrajectoryId = wellboreTrajectoryId } );
        }
    }
}
