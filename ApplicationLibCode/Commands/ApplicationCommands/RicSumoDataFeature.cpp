/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024     Equinor ASA
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

#include "RicSumoDataFeature.h"

#include "RiaApplication.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"
#include "RiaPreferences.h"
#include "RiaPreferencesSystem.h"

#include "RifArrowTools.h"

#include "RiuMainWindow.h"

#include <QAction>
#include <QSettings>

CAF_CMD_SOURCE_INIT( RicSumoDataFeature, "RicSumoDataFeature" );

SimpleDialog::SimpleDialog( QWidget* parent )
    : QDialog( parent )
{
    setWindowTitle( "Simple Dialog" );

    QVBoxLayout* layout = new QVBoxLayout( this );

    label = new QLabel( "This is a simple dialog.", this );
    layout->addWidget( label );

    authButton = new QPushButton( "Authenticate", this );
    connect( authButton, &QPushButton::clicked, this, &SimpleDialog::onAuthClicked );
    layout->addWidget( authButton );

    assetsButton = new QPushButton( "Asset Names", this );
    connect( assetsButton, &QPushButton::clicked, this, &SimpleDialog::onAssetsClicked );
    layout->addWidget( assetsButton );

    casesButton = new QPushButton( "Cases", this );
    connect( casesButton, &QPushButton::clicked, this, &SimpleDialog::onCasesClicked );
    layout->addWidget( casesButton );

    vectorNamesButton = new QPushButton( "Vector Names", this );
    connect( vectorNamesButton, &QPushButton::clicked, this, &SimpleDialog::onVectorNamesClicked );
    layout->addWidget( vectorNamesButton );

    blobIdButton = new QPushButton( "Blob Id", this );
    connect( blobIdButton, &QPushButton::clicked, this, &SimpleDialog::onFindBlobIdClicked );
    layout->addWidget( blobIdButton );

    parquetDownloadButton = new QPushButton( "Parquet", this );
    connect( parquetDownloadButton, &QPushButton::clicked, this, &SimpleDialog::onParquetClicked );
    layout->addWidget( parquetDownloadButton );

    showContentParquetButton = new QPushButton( "Show Content Parquet", this );
    connect( showContentParquetButton, &QPushButton::clicked, this, &SimpleDialog::onShowContentParquetClicked );
    layout->addWidget( showContentParquetButton );

    realizationIdsButton = new QPushButton( "Realizations", this );
    connect( realizationIdsButton, &QPushButton::clicked, this, &SimpleDialog::onRealizationsClicked );
    layout->addWidget( realizationIdsButton );

    okButton = new QPushButton( "OK", this );
    connect( okButton, &QPushButton::clicked, this, &SimpleDialog::onOkClicked );
    layout->addWidget( okButton );

    cancelButton = new QPushButton( "Cancel", this );
    connect( cancelButton, &QPushButton::clicked, this, &SimpleDialog::onCancelClicked );
    layout->addWidget( cancelButton );

    setLayout( layout );

    m_sumoConnector = RiaApplication::instance()->makeSumoConnector();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
SimpleDialog::~SimpleDialog()
{
    if ( m_sumoConnector )
    {
        m_sumoConnector->deleteLater();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::createConnection()
{
    m_sumoConnector = RiaApplication::instance()->makeSumoConnector();
    connect( m_sumoConnector, &RiaSumoConnector::tokenReady, this, &SimpleDialog::onTokenReady );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onAuthClicked()
{
    createConnection();
    m_sumoConnector->requestToken();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onAssetsClicked()
{
    if ( !isTokenValid() ) return;

    m_sumoConnector->requestAssets();
    m_sumoConnector->assets();

    label->setText( "Requesting fields (see log for response" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onCasesClicked()
{
    if ( !isTokenValid() ) return;

    QString fieldName = "Drogon";
    m_sumoConnector->requestCasesForField( fieldName );

    label->setText( "Requesting cases (see log for response" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onVectorNamesClicked()
{
    if ( !isTokenValid() ) return;

    SumoCaseId caseId( "5b783aab-ce10-4b78-b129-baf8d8ce4baa" );
    QString    iteration = "iter-0";

    m_sumoConnector->requestVectorNamesForEnsemble( caseId, iteration );

    label->setText( "Requesting vector names (see log for response" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onFindBlobIdClicked()
{
    if ( !isTokenValid() ) return;

    SumoCaseId caseId( "5b783aab-ce10-4b78-b129-baf8d8ce4baa" );
    QString    iteration  = "iter-0";
    QString    vectorName = "FOPT";

    m_sumoConnector->requestBlobIdForEnsemble( caseId, iteration, vectorName );

    label->setText( "Requesting blob ID for vector name (see log for response" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onParquetClicked()
{
    if ( !isTokenValid() ) return;

    if ( m_sumoConnector->blobUrls().empty() )
    {
        onFindBlobIdClicked();
    }

    if ( !m_sumoConnector->blobUrls().empty() )
    {
        m_sumoConnector->requestBlobDownload( m_sumoConnector->blobUrls().back() );

        label->setText( "Requesting blob ID for vector name (see log for response" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onShowContentParquetClicked()
{
    if ( m_sumoConnector->blobContents().empty() ) return;

    auto blob = m_sumoConnector->blobContents().back();

    auto content = blob.contents;

    QFile file( "f:/scratch/parquet.parquet" );
    if ( file.open( QIODevice::WriteOnly ) )
    {
        file.write( blob.contents );
        file.close();
    }

    // TODO: show content using parquet reader
    auto tableText = RifArrowTools::readFirstRowsOfTable( content );
    RiaLogging::info( tableText );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onRealizationsClicked()
{
    if ( !isTokenValid() ) return;

    SumoCaseId caseId( "485041ce-ad72-48a3-ac8c-484c0ed95cf8" );
    QString    iteration = "iter-0";

    m_sumoConnector->requestRealizationIdsForEnsembleBlocking( caseId, iteration );

    auto ids = m_sumoConnector->realizationIds();
    for ( const auto& id : ids )
    {
        RiaLogging::info( id );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool SimpleDialog::isTokenValid()
{
    if ( !m_sumoConnector )
    {
        createConnection();
    }

    if ( m_sumoConnector->token().isEmpty() )
    {
        m_sumoConnector->requestToken();
    }

    return !m_sumoConnector->token().isEmpty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onTokenReady( const QString& token )
{
    RiaLogging::info( "Token ready: " + token );
}

void SimpleDialog::onOkClicked()
{
    qDebug( "OK button clicked" );
    accept();
}

void SimpleDialog::onCancelClicked()
{
    qDebug( "Cancel button clicked" );
    reject();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSumoDataFeature::onActionTriggered( bool isChecked )
{
    if ( !m_dialog )
    {
        m_dialog = new SimpleDialog( RiaGuiApplication::instance()->mainWindow() );
    }
    m_dialog->show();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSumoDataFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "SUMO" );
}
