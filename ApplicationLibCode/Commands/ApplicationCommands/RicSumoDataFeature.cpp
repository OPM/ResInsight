/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "../../FileInterface/RifArrowTools.h"
#include "Sumo/RimSumoConnector.h"

#include "RiuMainWindow.h"

#include <QAction>

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

    okButton = new QPushButton( "OK", this );
    connect( okButton, &QPushButton::clicked, this, &SimpleDialog::onOkClicked );
    layout->addWidget( okButton );

    cancelButton = new QPushButton( "Cancel", this );
    connect( cancelButton, &QPushButton::clicked, this, &SimpleDialog::onCancelClicked );
    layout->addWidget( cancelButton );

    setLayout( layout );

    m_sumoConnector = nullptr;
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
    const QString server    = "https://main-sumo-prod.radix.equinor.com";
    const QString authority = "https://login.microsoftonline.com/3aa4a235-b6e2-48d5-9195-7fcf05b459b0";
    const QString scopes    = "9e5443dd-3431-4690-9617-31eed61cb55a/.default";
    const QString clientId  = "d57a8f87-4e28-4391-84d6-34356d5876a2";

    m_sumoConnector = new RimSumoConnector( qApp, server, authority, scopes, clientId );

    connect( m_sumoConnector, &RimSumoConnector::tokenReady, this, &SimpleDialog::onTokenReady );

    // get token from log first time the token is requested

    QSettings settings;
    auto      bearerToken = settings.value( m_registryKeyBearerToken_DEBUG_ONLY ).toString();

    if ( !bearerToken.isEmpty() )
    {
        m_sumoConnector->setToken( bearerToken );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onAuthClicked()
{
    QSettings settings;
    settings.setValue( m_registryKeyBearerToken_DEBUG_ONLY, "" );

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

    QString caseId    = "5b783aab-ce10-4b78-b129-baf8d8ce4baa";
    QString iteration = "iter-0";

    m_sumoConnector->requestVectorNamesForEnsemble( caseId, iteration );

    label->setText( "Requesting vector names (see log for response" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onFindBlobIdClicked()
{
    if ( !isTokenValid() ) return;

    QString caseId     = "5b783aab-ce10-4b78-b129-baf8d8ce4baa";
    QString iteration  = "iter-0";
    QString vectorName = "FOPT";

    m_sumoConnector->requestBlobIdForEnsemble( caseId, iteration, vectorName );

    label->setText( "Requesting blob ID for vector name (see log for response" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void SimpleDialog::onParquetClicked()
{
    if ( !isTokenValid() ) return;

    if ( m_sumoConnector->blobIds().empty() )
    {
        onFindBlobIdClicked();
    }

    if ( !m_sumoConnector->blobIds().empty() )
    {
        m_sumoConnector->requestBlobDownload( m_sumoConnector->blobIds().back() );

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
    // TODO: show content using parquet reader
    auto tableText = RifArrowTools::readSummaryData_debug( content );
    RiaLogging::info( tableText );
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

    QSettings settings;
    settings.setValue( m_registryKeyBearerToken_DEBUG_ONLY, token );

    m_sumoConnector->setToken( token );
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
