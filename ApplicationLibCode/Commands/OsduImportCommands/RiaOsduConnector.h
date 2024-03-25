#include <QtCore>

#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

#include <map>

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

    void requestFieldsByName( const QString& token, const QString& fieldName );

public slots:
    void requestToken();
    void parseWells( QNetworkReply* reply );
    void parseWellTrajectory( QNetworkReply* reply );
    void saveFile( QNetworkReply* reply );
    void granted();

signals:
    void finished();
    void tokenReady( const QString& token );

private:
    void addStandardHeader( QNetworkRequest& networkRequest, const QString& token, const QString& dataPartitionId );

    QNetworkReply*
        makeRequest( const std::map<QString, QString>& parameters, const QString& server, const QString& dataPartitionId, const QString& token );

    QNetworkReply* makeDownloadRequest( const QString& server, const QString& dataPartitionId, const QString& id, const QString& token );

    void requestFieldsByName( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldName );
    void requestWellsByFieldId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fieldId );
    void requestWellboresByWellId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& wellId );
    void requestWellboreTrajectoryByWellboreId( const QString& server,
                                                const QString& dataPartitionId,
                                                const QString& token,
                                                const QString& wellboreId );
    void requestFileDownloadByFileId( const QString& server, const QString& dataPartitionId, const QString& token, const QString& fileId );

    static QString generateRandomString( int length = 20 );
    static QString constructSearchUrl( const QString& server );
    static QString constructDownloadUrl( const QString& server, const QString& fileId );
    static QString constructAuthUrl( const QString& authority );
    static QString constructTokenUrl( const QString& authority );

    QOAuth2AuthorizationCodeFlow* m_osdu;
    QNetworkAccessManager*        m_networkAccessManager;

    const QString m_server;
    const QString m_dataPartitionId;
    const QString m_authority;
    const QString m_scopes;
    const QString m_clientId;
};
