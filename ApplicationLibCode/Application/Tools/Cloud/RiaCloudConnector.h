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

#pragma once

#include <QAbstractOAuth>
#include <QObject>

class QNetworkAccessManager;
class QOAuth2AuthorizationCodeFlow;

//==================================================================================================
///
//==================================================================================================
class RiaCloudConnector : public QObject
{
    Q_OBJECT
public:
    RiaCloudConnector( QObject*       parent,
                       const QString& server,
                       const QString& authority,
                       const QString& scopes,
                       const QString& clientId,
                       unsigned int   port );
    ~RiaCloudConnector() override;

    QString token() const;

    void importTokenFromFile();
    void exportTokenToFile();

    void setTokenDataFilePath( const QString& filePath );

    QString server() const;

    QString requestTokenBlocking();

public slots:
    void requestToken();
    void requestFailed( const QAbstractOAuth::Error error );

signals:
    void tokenReady( const QString& token );

private slots:
    void errorReceived( const QString& error, const QString& errorDescription, const QUrl& uri );
    void authorizationCallbackReceived( const QVariantMap& data );
    void accessGranted();

protected:
    static QString constructAuthUrl( const QString& authority );
    static QString constructTokenUrl( const QString& authority );

    QOAuth2AuthorizationCodeFlow* m_authCodeFlow;
    QNetworkAccessManager*        m_networkAccessManager;

    const QString m_server;
    const QString m_authority;
    const QString m_scopes;
    const QString m_clientId;

    QString m_tokenDataFilePath;
};
