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

#include "RiaCloudConnector.h"

#include "RiaCloudDefines.h"
#include "RiaConnectorTools.h"
#include "RiaLogging.h"
#include "RiaOAuthHttpServerReplyHandler.h"

#include <QDateTime>
#include <QDesktopServices>
#include <QDialog>
#include <QEventLoop>
#include <QLabel>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QPushButton>
#include <QTimer>
#include <QUrlQuery>
#include <QVBoxLayout>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCloudConnector::RiaCloudConnector( QObject*       parent,
                                      const QString& server,
                                      const QString& authority,
                                      const QString& scopes,
                                      const QString& clientId,
                                      unsigned int   port )
    : QObject( parent )
    , m_server( server )
    , m_authority( authority )
    , m_scopes( scopes )
    , m_clientId( clientId )
{
    m_authCodeFlow         = new QOAuth2AuthorizationCodeFlow( this );
    m_networkAccessManager = new QNetworkAccessManager( this );
    m_authCodeFlow->setNetworkAccessManager( m_networkAccessManager );

    RiaLogging::debug( "SSL BUILD VERSION: " + QSslSocket::sslLibraryBuildVersionString() );
    RiaLogging::debug( "SSL VERSION STRING: " + QSslSocket::sslLibraryVersionString() );

    RiaLogging::debug( "Cloud config:" );
    RiaLogging::debug( "  server: '" + server + "'" );
    RiaLogging::debug( "  authority: '" + authority + "'" );
    RiaLogging::debug( "  scopes: '" + scopes + "'" );
    RiaLogging::debug( "  client id: '" + clientId + "'" );

    connect( m_authCodeFlow,
             &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
             []( QUrl url )
             {
                 RiaLogging::info( "Authorize with url: " + url.toString() );
                 QUrlQuery query( url );
                 url.setQuery( query );
                 QDesktopServices::openUrl( url );
             } );

    QString authUrl = constructAuthUrl( m_authority );
    m_authCodeFlow->setAuthorizationUrl( QUrl( authUrl ) );

    QString tokenUrl = constructTokenUrl( m_authority );
    m_authCodeFlow->setAccessTokenUrl( QUrl( tokenUrl ) );

    // App key
    m_authCodeFlow->setClientIdentifier( m_clientId );
    m_authCodeFlow->setScope( m_scopes );

    auto replyHandler = new RiaOAuthHttpServerReplyHandler( port, this );
    m_authCodeFlow->setReplyHandler( replyHandler );
    RiaLogging::debug( "Server callback: " + replyHandler->callback() );

    connect( m_authCodeFlow, SIGNAL( granted() ), this, SLOT( accessGranted() ) );

    connect( m_authCodeFlow,
             SIGNAL( error( const QString&, const QString&, const QUrl& ) ),
             this,
             SLOT( errorReceived( const QString&, const QString&, const QUrl& ) ) );

    connect( m_authCodeFlow,
             SIGNAL( authorizationCallbackReceived( const QVariantMap& ) ),
             this,
             SLOT( authorizationCallbackReceived( const QVariantMap& ) ) );

    connect( m_authCodeFlow,
             &QOAuth2AuthorizationCodeFlow::tokenChanged,
             this,
             [&]( const QString& token )
             {
                 RiaLogging::debug( "Access token changed." );
                 exportTokenToFile();
             } );

    connect( m_authCodeFlow,
             &QOAuth2AuthorizationCodeFlow::refreshTokenChanged,
             this,
             [&]( const QString& refreshToken )
             {
                 RiaLogging::debug( "Refresh token changed." );
                 exportTokenToFile();
             } );

    connect( m_authCodeFlow,
             &QOAuth2AuthorizationCodeFlow::expirationAtChanged,
             this,
             [&]( const QDateTime& expiration )
             {
                 RiaLogging::debug( QString( "Access token expiration changed: %1" ).arg( expiration.toString() ) );
                 exportTokenToFile();
             } );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCloudConnector::~RiaCloudConnector()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::accessGranted()
{
    QString currentToken = m_authCodeFlow->token();
    emit    tokenReady( currentToken );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::errorReceived( const QString& error, const QString& errorDescription, const QUrl& uri )
{
    RiaLogging::debug( "Cloud Error Received: " + error + ". Description: " + errorDescription );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::authorizationCallbackReceived( const QVariantMap& data )
{
    RiaLogging::debug( "Authorization callback received:" );
    for ( const auto& [key, value] : data.toStdMap() )
    {
        RiaLogging::debug( "  Key: " + key + " Value: " + value.toString() );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::requestFailed( const QAbstractOAuth::Error error )
{
    RiaLogging::error( "Request failed: " );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::requestToken()
{
    if ( token().isEmpty() )
    {
        RiaLogging::debug( "No valid access token found." );
        if ( !m_authCodeFlow->refreshToken().isEmpty() )
        {
            RiaLogging::info( "Refreshing access token with refresh token." );
            m_authCodeFlow->refreshAccessToken();
        }
        else
        {
            RiaLogging::info( "Requesting token." );
            m_authCodeFlow->grant();
        }
    }
    else
    {
        RiaLogging::debug( "Has token: skipping token request." );
        emit accessGranted();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudConnector::token() const
{
    QString   currentToken = m_authCodeFlow->token();
    QDateTime expiration   = m_authCodeFlow->expirationAt();
    if ( !currentToken.isEmpty() && expiration.isValid() && expiration > QDateTime::currentDateTime() )
    {
        return currentToken;
    }
    else
    {
        return QString();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::exportTokenToFile()
{
    QString tokenDataJson = RiaConnectorTools::tokenDataAsJson( m_authCodeFlow );
    RiaConnectorTools::writeTokenData( m_tokenDataFilePath, tokenDataJson );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::importTokenFromFile()
{
    auto tokenDataJson = RiaConnectorTools::readStringFromFile( m_tokenDataFilePath );
    if ( !tokenDataJson.isEmpty() )
    {
        RiaConnectorTools::initializeTokenDataFromJson( m_authCodeFlow, tokenDataJson );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::setTokenDataFilePath( const QString& filePath )
{
    m_tokenDataFilePath = filePath;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::clearTokens()
{
    if ( m_authCodeFlow )
    {
        m_authCodeFlow->setToken( "" );
        m_authCodeFlow->setRefreshToken( "" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaCloudConnector::isGranted() const
{
    return m_authCodeFlow->status() == QAbstractOAuth::Status::Granted;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudConnector::constructAuthUrl( const QString& authority )
{
    return authority + "/oauth2/v2.0/authorize";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudConnector::constructTokenUrl( const QString& authority )
{
    return authority + "/oauth2/v2.0/token";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudConnector::requestTokenBlocking()
{
    QString currentToken = token();
    if ( !currentToken.isEmpty() ) return currentToken;

    QTimer timer;
    timer.setSingleShot( true );
    QEventLoop loop;
    connect( this, SIGNAL( tokenReady( const QString& ) ), &loop, SLOT( quit() ) );
    connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
    requestToken();
    timer.start( RiaCloudDefines::requestTokenTimeoutMillis() );
    loop.exec( QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents );

    return m_authCodeFlow->token();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::requestTokenWithCancelButton()
{
    QDialog dialog;
    dialog.setWindowTitle( "Requesting Token" );
    dialog.setModal( true );

    QVBoxLayout* layout = new QVBoxLayout( &dialog );

    QLabel* label =
        new QLabel( "Requesting token.\nIf this process takes a long time,\nissue a new authentication process by pressing\n'New "
                    "Authentication'\n\n Please wait..." );
    label->setAlignment( Qt::AlignHCenter );
    layout->addWidget( label );

    // Create horizontal button layout
    QHBoxLayout* buttonLayout = new QHBoxLayout();

    // Add buttons to horizontal layout
    QPushButton* requestNewAuthenticationButton = new QPushButton( "New Authentication" );
    QPushButton* cancelButton                   = new QPushButton( "Cancel" );
    buttonLayout->addWidget( requestNewAuthenticationButton );
    buttonLayout->addWidget( cancelButton );

    // Add button layout to main layout
    layout->addLayout( buttonLayout );

    QTimer timer;
    timer.setSingleShot( true );

    QEventLoop loop;

    connect( this, &RiaCloudConnector::tokenReady, &loop, &QEventLoop::quit );
    connect( &timer, &QTimer::timeout, &loop, &QEventLoop::quit );
    connect( cancelButton,
             &QPushButton::clicked,
             [&]()
             {
                 RiaLogging::info( "Token request canceled by user." );
                 timer.stop();
                 loop.quit();
             } );

    connect( requestNewAuthenticationButton,
             &QPushButton::clicked,
             [&]()
             {
                 clearTokens();
                 requestToken();
             } );

    connect( &dialog,
             &QDialog::rejected,
             [&]()
             {
                 timer.stop();
                 loop.quit();
             } );

    requestToken();
    timer.start( RiaCloudDefines::requestTokenTimeoutMillis() );

    dialog.show();
    loop.exec();
    dialog.close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudConnector::server() const
{
    return m_server;
}
