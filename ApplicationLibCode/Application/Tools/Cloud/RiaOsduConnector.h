/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2024- Equinor ASA
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

#include <QtCore>

#include <QNetworkAccessManager>
#include <QtNetworkAuth/QOAuth2AuthorizationCodeFlow>

#include <map>
#include <optional>

struct OsduField
{
    QString id;
    QString kind;
    QString name;
};

struct OsduWellbore
{
    QString id;
    QString kind;
    QString name;
    QString wellId;
    QString fieldId;
    double  datumElevation;
};

struct OsduWellboreTrajectory
{
    QString id;
    QString kind;
    QString wellboreId;
};

struct OsduWellLogChannel
{
    QString id;
    QString mnemonic;
    QString description;
    double  topDepth;
    double  baseDepth;
    QString interpreterName;
    QString quality;
    QString unit;
    QString depthUnit;
};

struct OsduWellLog
{
    QString                         id;
    QString                         kind;
    QString                         name;
    QString                         description;
    double                          samplingStart;
    double                          samplingStop;
    QString                         wellboreId;
    std::vector<OsduWellLogChannel> channels;
};

//==================================================================================================
///
//==================================================================================================
class RiaOsduConnector : public RiaCloudConnector
{
    Q_OBJECT
public:
    RiaOsduConnector( QObject*       parent,
                      const QString& server,
                      const QString& dataParitionId,
                      const QString& authority,
                      const QString& scopes,
                      const QString& clientId,
                      unsigned int   port );
    ~RiaOsduConnector() override;

    void                     requestFieldsByName( const QString& fieldName );
    void                     requestWellboresByFieldId( const QString& fieldId );
    void                     requestWellboreTrajectoryByWellboreId( const QString& wellboreId );
    void                     requestWellLogsByWellboreId( const QString& wellboreId );
    std::vector<OsduWellLog> requestWellLogsByWellboreIdBlocking( const QString& wellboreId );

    void requestWellLogParquetDataById( const QString& wellLogId );
    void requestWellboreTrajectoryParquetDataById( const QString& wellboreTrajectoryId );

    std::pair<QByteArray, QString> requestWellLogParquetDataByIdBlocking( const QString& wellLogId );
    std::pair<QByteArray, QString> requestWellboreTrajectoryParquetDataByIdBlocking( const QString& wellboreTrajectoryId );

    std::optional<OsduWellbore> wellboreById( const QString& wellboreId ) const;

    void cancelRequestForId( const QString& id );

    void clearCachedData();

    QString dataPartition() const;

    std::vector<OsduField>              fields() const;
    std::vector<OsduWellbore>           wellboresByFieldId( const QString& fieldId ) const;
    std::vector<OsduWellboreTrajectory> wellboreTrajectories( const QString& wellboreId ) const;
    std::vector<OsduWellLog>            wellLogs( const QString& wellboreId ) const;

public slots:
    void parseFields( QNetworkReply* reply );
    void parseWellboresByFieldId( QNetworkReply* reply, const QString& fieldId );
    void parseWellTrajectory( QNetworkReply* reply, const QString& wellboreId );
    void parseWellLogs( QNetworkReply* reply, const QString& wellboreId );
    void parquetDownloadComplete( const QByteArray&, const QString& url, const QString& id );

signals:
    void parquetDownloadFinished( const QByteArray& contents, const QString& url, const QString& id );
    void fieldsFinished();
    void wellboresByFieldIdFinished( const QString& fieldId );
    void wellboreTrajectoryFinished( const QString& wellboreId, int numTrajectories, const QString& errorMessage );
    void wellLogsFinished( const QString& wellboreId );

private slots:
    void requestParquetData( const QString& url, const QString& dataPartitionId, const QString& token, const QString& id );

private:
    void addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& dataPartitionId, const QString& contentType );

    QNetworkReply* makeSearchRequest( const std::map<QString, QString>& parameters,
                                      const QString&                    server,
                                      const QString&                    dataPartitionId,
                                      const QString&                    token );

    QNetworkReply* makeDownloadRequest( const QString& url, const QString& dataPartitionId, const QString& token, const QString& contentType );

    void requestFieldsByName( const QString& token, const QString& fieldName );
    void requestFieldsByName( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldName );
    void requestWellboresByFieldId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldId );
    void requestWellboreTrajectoryByWellboreId( const QString& server,
                                                const QString& dataPartitionId,
                                                const QString& token,
                                                const QString& wellboreId );
    void requestWellLogsByWellboreId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& wellboreId );

    static QString constructSearchUrl( const QString& server );
    static QString constructFileDownloadUrl( const QString& server, const QString& fileId );
    static QString constructWellLogDownloadUrl( const QString& server, const QString& wellLogId );
    static QString constructWellboreTrajectoriesDownloadUrl( const QString& server, const QString& wellboreTrajectoryId );

    std::pair<QByteArray, QString> requestParquetDataByUrlBlocking( const QString& url, const QString& id );
    void                           requestParquetDataByUrl( const QString& url, const QString& id );

    const QString m_dataPartitionId;

    mutable QMutex                                         m_mutex;
    mutable QMutex                                         m_repliesMutex;
    std::vector<OsduField>                                 m_fields;
    std::map<QString, std::vector<OsduWellbore>>           m_wellbores;
    std::map<QString, std::vector<OsduWellboreTrajectory>> m_wellboreTrajectories;
    std::map<QString, std::vector<OsduWellLog>>            m_wellLogs;
    std::map<QString, QByteArray>                          m_parquetData;
    std::map<QString, QString>                             m_parquetErrors;
    std::map<QString, QPointer<QNetworkReply>>             m_replies;
};
