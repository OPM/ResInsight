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

#include "RiaCloudConnector.h"
#include "RiaSumoDefines.h"

#include <QByteArray>
#include <QNetworkAccessManager>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>

#include <map>

class QEventLoop;

using SumoObjectId = QString;

struct SumoAsset
{
    SumoAssetId assetId;

    QString kind;
    QString name;
};

struct SumoCase
{
    SumoCaseId caseId;

    QString kind;
    QString name;
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

struct SumoEnsemble
{
    SumoCaseId caseId;
    QString    name;
};

//==================================================================================================
///
//==================================================================================================
class RiaSumoConnector : public RiaCloudConnector
{
    Q_OBJECT
public:
    RiaSumoConnector( QObject*       parent,
                      const QString& server,
                      const QString& authority,
                      const QString& scopes,
                      const QString& clientId,
                      unsigned int   port );
    ~RiaSumoConnector() override;

    void requestAssets();
    void requestAssetsBlocking();

    void requestCasesForField( const QString& fieldName );
    void requestCasesForFieldBlocking( const QString& fieldName );

    void requestEnsembleByCasesId( const SumoCaseId& caseId );
    void requestEnsembleByCasesIdBlocking( const SumoCaseId& caseId );

    void requestVectorNamesForEnsemble( const SumoCaseId& caseId, const QString& ensembleName );
    void requestVectorNamesForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName );

    void requestRealizationIdsForEnsemble( const SumoCaseId& caseId, const QString& ensembleName );
    void requestRealizationIdsForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName );

    void       requestParametersBlobIdForEnsemble( const SumoCaseId& caseId, const QString& ensembleName );
    void       requestParametersBlobIdForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName );
    QByteArray requestParametersParquetDataBlocking( const SumoCaseId& caseId, const QString& ensembleName );

    void requestBlobIdForEnsemble( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );
    void requestBlobIdForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );

    void requestBlobDownload( const QString& blobId );
    void requestBlobByRedirectUri( const QString& blobId, const QString& redirectUri );

    QByteArray requestParquetDataBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );

    std::vector<SumoAsset>    assets() const;
    std::vector<SumoCase>     cases() const;
    std::vector<QString>      ensembleNamesForCase( const SumoCaseId& caseId ) const;
    std::vector<QString>      vectorNames() const;
    std::vector<QString>      realizationIds() const;
    std::vector<QString>      blobUrls() const;
    std::vector<SumoRedirect> blobContents() const;

public slots:
    void parseAssets( QNetworkReply* reply );
    void parseEnsembleNames( QNetworkReply* reply, const SumoCaseId& caseId );
    void parseCases( QNetworkReply* reply );
    void parseVectorNames( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName );
    void parseRealizationNumbers( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName );
    void parseBlobIds( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName, bool isParameters );

    void requestFailed( const QAbstractOAuth::Error error );
    void parquetDownloadComplete( const QString& blobId, const QByteArray&, const QString& url );

signals:
    void fileDownloadFinished( const QString& fileId, const QString& filePath );
    void casesFinished();
    void wellsFinished();
    void wellboresFinished( const QString& wellId );
    void wellboreTrajectoryFinished( const QString& wellboreId );
    void parquetDownloadFinished( const QByteArray& contents, const QString& url );
    void ensembleNamesFinished();
    void vectorNamesFinished();
    void blobIdFinished();
    void assetsFinished();
    void realizationIdsFinished();

private:
    void addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& contentType );

    QNetworkReply* makeRequest( const std::map<QString, QString>& parameters, const QString& server, const QString& token );
    QNetworkReply* makeDownloadRequest( const QString& url, const QString& token, const QString& contentType );
    void           requestParquetData( const QString& url, const QString& token );

    static QString constructSearchUrl( const QString& server );
    static QString constructDownloadUrl( const QString& server, const QString& blobId );

    void wrapAndCallNetworkRequest( std::function<void()> requestCallable, const QMetaMethod& signalMethod );

private:
    std::vector<SumoAsset>    m_assets;
    std::vector<SumoCase>     m_cases;
    std::vector<QString>      m_vectorNames;
    std::vector<QString>      m_realizationIds;
    std::vector<SumoEnsemble> m_ensembleNames;

    std::vector<QString> m_blobUrl;

    std::vector<SumoRedirect> m_redirectInfo;
};
