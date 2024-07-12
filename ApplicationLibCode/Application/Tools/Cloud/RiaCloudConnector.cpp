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

#include "RiaConnectorTools.h"
#include "RiaLogging.h"
#include "RiaOsduDefines.h"
#include "RiaOsduOAuthHttpServerReplyHandler.h"
#include "RiaSumoDefines.h"

#include <QAbstractOAuth>
#include <QDesktopServices>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOAuth2AuthorizationCodeFlow>
#include <QOAuthHttpServerReplyHandler>
#include <QObject>
#include <QString>
#include <QTimer>
#include <QUrl>
#include <QUrlQuery>
#include <QtCore>

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

    auto replyHandler = new RiaOsduOAuthHttpServerReplyHandler( port, this );
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
    m_token = m_authCodeFlow->token();

    QString tokenDataJson = RiaConnectorTools::tokenDataAsJson( m_authCodeFlow );
    RiaConnectorTools::writeTokenData( m_tokenDataFilePath, tokenDataJson );

    emit tokenReady( m_token );
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
    RiaLogging::debug( "Requesting token." );
    m_authCodeFlow->grant();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudConnector::token() const
{
    return m_token;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaCloudConnector::importTokenFromFile()
{
    auto tokenDataJson = RiaConnectorTools::readTokenData( m_tokenDataFilePath );
    if ( !tokenDataJson.isEmpty() )
    {
        RiaConnectorTools::initializeTokenDataFromJson( m_authCodeFlow, tokenDataJson );
        m_token = m_authCodeFlow->token();
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
    if ( !m_token.isEmpty() ) return m_token;

    QTimer timer;
    timer.setSingleShot( true );
    QEventLoop loop;
    connect( this, SIGNAL( tokenReady( const QString& ) ), &loop, SLOT( quit() ) );
    connect( &timer, SIGNAL( timeout() ), &loop, SLOT( quit() ) );
    requestToken();
    timer.start( RiaSumoDefines::requestTimeoutMillis() );
    loop.exec();
    return m_token;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaCloudConnector::server() const
{
    return m_server;
}
