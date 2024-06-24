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

#include <QtCore>

#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

#include <map>

using SumoObjectId = QString;

struct SumoAsset
{
    SumoObjectId id;
    QString      kind;
    QString      name;
};

struct SumoCase
{
    SumoObjectId id;
    QString      kind;
    QString      name;
};

struct SumoRedirect
{
    SumoObjectId objectId;
    QString      blobName;
    QString      url;
    QString      redirectBaseUri;
    QString      redirectAuth;
    QByteArray   contents;
};

//==================================================================================================
///
//==================================================================================================
class RimSumoConnector : public QObject
{
    Q_OBJECT
public:
    RimSumoConnector( QObject* parent, const QString& server, const QString& authority, const QString& scopes, const QString& clientId );
    ~RimSumoConnector() override;

    void    setToken( const QString& token );
    QString token() const;

    void requestCasesForField( const QString& fieldName );
    void requestAssets();

    void requestEnsembleByCasesId( const QString& vectorName, const QString& caseId );
    void requestVectorNamesForEnsemble( const QString& caseId, const QString& ensembleName );
    void requestBlobIdForEnsemble( const QString& caseId, const QString& ensembleName, const QString& vectorName );
    void requestBlobDownload( const QString& blobId );
    void requestBlobByRedirectUri( const QString& blobId, const QString& redirectUri );

    QString server() const;

    std::vector<SumoAsset>    assets() const;
    std::vector<SumoCase>     cases() const;
    std::vector<QString>      vectorNames() const;
    std::vector<QString>      blobUrls() const;
    std::vector<QString>      blobIds() const;
    std::vector<SumoRedirect> blobContents() const;

public slots:
    void requestToken();

    void parseAssets( QNetworkReply* reply );
    void parseCases( QNetworkReply* reply );
    void parseVectorNames( QNetworkReply* reply, const QString& caseId, const QString& ensembleName );
    void parseBlobIds( QNetworkReply* reply, const QString& caseId, const QString& ensembleName, const QString& vectorName );

    void saveFile( QNetworkReply* reply, const QString& fileId );

    void accessGranted();
    void requestFailed( const QAbstractOAuth::Error error );
    void parquetDownloadComplete( const QString& blobId, const QByteArray&, const QString& url );

signals:
    void fileDownloadFinished( const QString& fileId, const QString& filePath );
    void casesFinished();
    void wellsFinished();
    void wellboresFinished( const QString& wellId );
    void wellboreTrajectoryFinished( const QString& wellboreId );
    void tokenReady( const QString& token );
    void parquetDownloadFinished( const QByteArray& contents, const QString& url );

private:
    void addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& contentType );

    QNetworkReply* makeRequest( const std::map<QString, QString>& parameters, const QString& server, const QString& token );
    QNetworkReply* makeDownloadRequest( const QString& url, const QString& token, const QString& contentType );
    void           requestParquetData( const QString& url, const QString& token );

    static QString generateRandomString( int length = 20 );
    static QString constructSearchUrl( const QString& server );
    static QString constructDownloadUrl( const QString& server, const QString& blobId );
    static QString constructAuthUrl( const QString& authority );
    static QString constructTokenUrl( const QString& authority );

    QOAuth2AuthorizationCodeFlow* m_authCodeFlow;
    QNetworkAccessManager*        m_networkAccessManager;

    const QString m_server;
    const QString m_authority;
    const QString m_scopes;
    const QString m_clientId;

    QString m_token;

    std::vector<SumoAsset> m_assets;
    std::vector<SumoCase>  m_cases;
    std::vector<QString>   m_vectorNames;

    std::vector<QString> m_blobName;
    std::vector<QString> m_blobUrl;

    QString m_redirect;

    std::vector<SumoRedirect> m_redirectInfo;

    QByteArray m_parquetData;
};
