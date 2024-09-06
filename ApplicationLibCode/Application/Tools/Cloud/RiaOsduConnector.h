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

struct OsduField
{
    QString id;
    QString kind;
    QString name;
};

struct OsduWell
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
    void                     requestWellsByFieldId( const QString& fieldId );
    void                     requestWellboresByWellId( const QString& wellId );
    void                     requestWellboreTrajectoryByWellboreId( const QString& wellboreId );
    void                     requestWellLogsByWellboreId( const QString& wellboreId );
    std::vector<OsduWellLog> requestWellLogsByWellboreIdBlocking( const QString& wellboreId );

    void requestWellLogParquetDataById( const QString& wellLogId );
    void requestWellboreTrajectoryParquetDataById( const QString& wellboreTrajectoryId );

    std::pair<QByteArray, QString> requestWellLogParquetDataByIdBlocking( const QString& wellLogId );
    std::pair<QByteArray, QString> requestWellboreTrajectoryParquetDataByIdBlocking( const QString& wellboreTrajectoryId );

    QString wellIdForWellboreId( const QString& wellboreId ) const;

    void cancelRequestForId( const QString& id );

    void clearCachedData();

    QString dataPartition() const;

    std::vector<OsduField>              fields() const;
    std::vector<OsduWell>               wells() const;
    std::vector<OsduWellbore>           wellbores( const QString& wellId ) const;
    std::vector<OsduWellboreTrajectory> wellboreTrajectories( const QString& wellboreId ) const;
    std::vector<OsduWellLog>            wellLogs( const QString& wellboreId ) const;

public slots:
    void parseFields( QNetworkReply* reply );
    void parseWells( QNetworkReply* reply );
    void parseWellbores( QNetworkReply* reply, const QString& wellId );
    void parseWellTrajectory( QNetworkReply* reply, const QString& wellboreId );
    void parseWellLogs( QNetworkReply* reply, const QString& wellboreId );
    void parquetDownloadComplete( const QByteArray&, const QString& url, const QString& id );

signals:
    void parquetDownloadFinished( const QByteArray& contents, const QString& url, const QString& id );
    void fieldsFinished();
    void wellsFinished();
    void wellboresFinished( const QString& wellId );
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
    void requestWellsByFieldId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldId );
    void requestWellboresByWellId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& wellId );
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
    std::vector<OsduWell>                                  m_wells;
    std::map<QString, std::vector<OsduWellbore>>           m_wellbores;
    std::map<QString, std::vector<OsduWellboreTrajectory>> m_wellboreTrajectories;
    std::map<QString, std::vector<OsduWellLog>>            m_wellLogs;
    std::map<QString, QByteArray>                          m_parquetData;
    std::map<QString, QString>                             m_parquetErrors;
    std::map<QString, QPointer<QNetworkReply>>             m_replies;
};
