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

#include <functional>
#include <list>
#include <map>

class QEventLoop;
class QThread;

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

struct SumoGridInfo
{
    QString          name;
    std::vector<int> realizations;
};

struct SumoGridPropertyInfo
{
    QString name;

    // Empty for a static property. For a dynamic property this is either a single timestamp ("2018-01-01")
    // or an interval ("2018-01-01/2019-01-01"). ResInsight currently only supports the single-timestamp form.
    QString isoDateOrInterval;
};

//==================================================================================================
///
//==================================================================================================
class RiaSumoConnector : public RiaCloudConnector
{
    Q_OBJECT
public:
    // The Sumo data is served by a server whose address is not known when the connector is constructed,
    // and which can change while the connector is alive. It is therefore supplied as a callable that is
    // invoked for every request, rather than as a fixed address.
    RiaSumoConnector( QObject*                 parent,
                      std::function<QString()> serverUrlProvider,
                      const QString&           authority,
                      const QString&           scopes,
                      const QString&           clientId,
                      unsigned int             port );
    ~RiaSumoConnector() override;

    QString server() const override;

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
    void requestBlobBySasUri( const QString& blobId, const QString& sasUri );

    QByteArray requestParquetDataBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName );

    void requestGridInfoForEnsemble( const SumoCaseId& caseId, const QString& ensembleName );
    void requestGridInfoForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName );

    void requestGridBlobIdForEnsemble( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );
    void requestGridBlobIdForEnsembleBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    QByteArray requestGridDataBlocking( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );

    void requestGridPropertyInfoForEnsemble( const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );
    void requestGridPropertyInfoForEnsembleBlocking( const SumoCaseId& caseId,
                                                     const QString&    ensembleName,
                                                     const QString&    gridName,
                                                     int               realization );

    QString requestGridPropertyBlobIdBlocking( const SumoCaseId& caseId,
                                               const QString&    ensembleName,
                                               const QString&    gridName,
                                               int               realization,
                                               const QString&    propertyName,
                                               const QString&    isoDateOrInterval );

    QByteArray requestGridPropertyDataBlocking( const SumoCaseId& caseId,
                                                const QString&    ensembleName,
                                                const QString&    gridName,
                                                int               realization,
                                                const QString&    propertyName,
                                                const QString&    isoDateOrInterval );

    // Download several time steps of one grid property concurrently and put them in the blob cache, so the
    // following per time step requests are served without going to Sumo. Entries already cached are skipped.
    void prefetchGridPropertyDataBlocking( const SumoCaseId&           caseId,
                                           const QString&              ensembleName,
                                           const QString&              gridName,
                                           int                         realization,
                                           const QString&              propertyName,
                                           const std::vector<QString>& isoDatesOrIntervals );

    std::vector<SumoAsset>            assets() const;
    std::vector<SumoCase>             cases() const;
    std::vector<QString>              ensembleNamesForCase( const SumoCaseId& caseId ) const;
    std::vector<QString>              vectorNames() const;
    std::vector<QString>              realizationIds() const;
    std::vector<SumoGridInfo>         gridInfos() const;
    std::vector<SumoGridPropertyInfo> gridPropertyInfos() const;
    std::vector<QString>              blobIds() const;
    std::vector<SumoRedirect>         blobContents() const;

public slots:
    void parseAssets( QNetworkReply* reply );
    void parseEnsembleNames( QNetworkReply* reply, const SumoCaseId& caseId );
    void parseCases( QNetworkReply* reply );
    void parseVectorNames( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName );
    void parseRealizationNumbers( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName );
    void parseGridInfo( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName );
    void parseGridPropertyInfo( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName, const QString& gridName, int realization );
    void parseBlobId( QNetworkReply* reply, const SumoCaseId& caseId, const QString& ensembleName, const QString& vectorName, bool isParameters );

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
    void gridInfoFinished();
    void gridPropertyInfoFinished();

private:
    void addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& contentType );

    QNetworkReply* makeDownloadRequest( const QString& url, const QString& token, const QString& contentType );
    void           requestParquetData( const QString& url, const QString& token );

    static QString constructSasUri( const QString& blobStoreBaseUri, const QString& blobId, const QString& sasToken );

    void wrapAndCallNetworkRequest( std::function<void()> requestCallable, const QMetaMethod& signalMethod );
    void waitForRequest( const std::function<void()>& requestCallable, const QMetaMethod& signalMethod );

    QByteArray gridPropertyBlobFromCache( const QString& cacheKey );
    void       insertGridPropertyBlobInCache( const QString& cacheKey, const QByteArray& contents );

    static QString gridPropertyCacheKey( const SumoCaseId& caseId,
                                         const QString&    ensembleName,
                                         const QString&    gridName,
                                         int               realization,
                                         const QString&    propertyName,
                                         const QString&    isoDateOrInterval );

    QNetworkReply* makeGridPropertyBlobIdRequest( const SumoCaseId& caseId,
                                                  const QString&    ensembleName,
                                                  const QString&    gridName,
                                                  int               realization,
                                                  const QString&    propertyName,
                                                  const QString&    isoDateOrInterval );

    static QString blobIdFromReply( QNetworkReply* reply, const QString& propertyName );

    void fetchGridPropertyBatch( const SumoCaseId&           caseId,
                                 const QString&              ensembleName,
                                 const QString&              gridName,
                                 int                         realization,
                                 const QString&              propertyName,
                                 const std::vector<QString>& timestampsToFetch,
                                 const std::vector<QString>& cacheKeys );

    static void waitForRepliesToFinish( const std::vector<QNetworkReply*>& replies );

    // Download one blob by id and return its contents, waiting on the transfer thread.
    QByteArray downloadBlobBlocking( const QString& blobId );

    // The network manager belonging to the calling thread: the transfer thread manager when called from
    // there, otherwise the one owned by RiaCloudConnector on the GUI thread.
    QNetworkAccessManager* networkAccessManager();

    // Run work on the transfer thread and block the caller until it returns. The caller waits on a semaphore
    // and dispatches no events, so nothing can re-enter the code that started the request. Called from the
    // transfer thread itself, the work is run directly, which keeps nested blocking requests working.
    void runOnTransferThreadBlocking( const std::function<void()>& work );

private:
    std::function<QString()> m_serverUrlProvider;

    std::vector<SumoAsset>    m_assets;
    std::vector<SumoCase>     m_cases;
    std::vector<QString>      m_vectorNames;
    std::vector<QString>      m_realizationIds;
    std::vector<SumoEnsemble> m_ensembleNames;
    std::vector<SumoGridInfo> m_gridInfos;

    std::vector<SumoGridPropertyInfo> m_gridPropertyInfos;

    std::vector<QString> m_blobId;

    std::vector<SumoRedirect> m_redirectInfo;

    // Downloaded grid-property blobs, keyed by the full property identity (case, ensemble, grid, realization,
    // property, timestamp). Displaying a property computes its global legend range across all time steps, so a
    // property is requested repeatedly; the cache ensures each blob is fetched from Sumo at most once.
    //
    // Bounded by total byte size, not by entry count, as the blob size follows the grid size and varies by orders
    // of magnitude. The least recently used entries are evicted when the limit is exceeded.
    struct GridPropertyBlobCacheEntry
    {
        QByteArray                   contents;
        std::list<QString>::iterator orderIterator;
    };

    std::map<QString, GridPropertyBlobCacheEntry> m_gridPropertyBlobCache;
    std::list<QString>                            m_gridPropertyBlobCacheOrder; // most recently used at front
    size_t                                        m_gridPropertyBlobCacheSizeBytes = 0;

    // Transfers run on their own thread so the calling thread can wait without dispatching events. Waiting on
    // a nested event loop on the GUI thread let the view update code re-enter a load that was still running,
    // and the same grid property was downloaded twice. Authentication stays on the GUI thread: the OAuth flow
    // opens a browser and its objects live there.
    QThread*               m_transferThread               = nullptr;
    QObject*               m_transferContext              = nullptr; // lives on the transfer thread
    QNetworkAccessManager* m_transferNetworkAccessManager = nullptr; // created on the transfer thread
};
