#pragma once

#include <QtCore>

#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

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

struct OsduWellLog
{
    QString id;
    QString kind;
    QString wellboreId;
};

//==================================================================================================
///
//==================================================================================================
class RiaOsduConnector : public QObject
{
    Q_OBJECT
public:
    RiaOsduConnector( QObject*       parent,
                      const QString& server,
                      const QString& dataParitionId,
                      const QString& authority,
                      const QString& scopes,
                      const QString& clientId );
    ~RiaOsduConnector() override;

    void                     requestFieldsByName( const QString& fieldName );
    void                     requestWellsByFieldId( const QString& fieldId );
    void                     requestWellboresByWellId( const QString& wellId );
    void                     requestWellboreTrajectoryByWellboreId( const QString& wellboreId );
    void                     requestWellLogsByWellboreId( const QString& wellboreId );
    std::vector<OsduWellLog> requestWellLogsByWellboreIdBlocking( const QString& wellboreId );

    std::pair<QByteArray, QString> requestWellLogParquetDataById( const QString& wellLogId );
    std::pair<QByteArray, QString> requestWellboreTrajectoryParquetDataById( const QString& wellboreTrajectoryId );

    QString wellIdForWellboreId( const QString& wellboreId ) const;

    void clearCachedData();

    QString server() const;
    QString dataPartition() const;

    std::vector<OsduField>              fields() const;
    std::vector<OsduWell>               wells() const;
    std::vector<OsduWellbore>           wellbores( const QString& wellId ) const;
    std::vector<OsduWellboreTrajectory> wellboreTrajectories( const QString& wellboreId ) const;
    std::vector<OsduWellLog>            wellLogs( const QString& wellboreId ) const;

public slots:
    void requestToken();
    void parseFields( QNetworkReply* reply );
    void parseWells( QNetworkReply* reply );
    void parseWellbores( QNetworkReply* reply, const QString& wellId );
    void parseWellTrajectory( QNetworkReply* reply, const QString& wellboreId );
    void parseWellLogs( QNetworkReply* reply, const QString& wellboreId );

    void accessGranted();
    void parquetDownloadComplete( const QByteArray&, const QString& url );

signals:
    void parquetDownloadFinished( const QByteArray& contents, const QString& url );
    void fieldsFinished();
    void wellsFinished();
    void wellboresFinished( const QString& wellId );
    void wellboreTrajectoryFinished( const QString& wellboreId, int numTrajectories, const QString& errorMessage );
    void wellLogsFinished( const QString& wellboreId );
    void tokenReady( const QString& token );

private:
    void requestParquetData( const QString& url, const QString& dataPartitionId, const QString& token );

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
    static QString constructAuthUrl( const QString& authority );
    static QString constructTokenUrl( const QString& authority );
    static QString constructWellLogDownloadUrl( const QString& server, const QString& wellLogId );
    static QString constructWellboreTrajectoriesDownloadUrl( const QString& server, const QString& wellboreTrajectoryId );

    std::pair<QByteArray, QString> requestParquetDataByUrl( const QString& url );

    QString requestTokenBlocking();

    QOAuth2AuthorizationCodeFlow* m_osdu;
    QNetworkAccessManager*        m_networkAccessManager;

    const QString m_server;
    const QString m_dataPartitionId;
    const QString m_authority;
    const QString m_scopes;
    const QString m_clientId;

    mutable QMutex                                         m_mutex;
    QString                                                m_token;
    std::vector<OsduField>                                 m_fields;
    std::vector<OsduWell>                                  m_wells;
    std::map<QString, std::vector<OsduWellbore>>           m_wellbores;
    std::map<QString, std::vector<OsduWellboreTrajectory>> m_wellboreTrajectories;
    std::map<QString, std::vector<OsduWellLog>>            m_wellLogs;
    QByteArray                                             m_parquetData;
};
