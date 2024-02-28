#include <QtCore>

#include <QNetworkAccessManager>
#include <QOAuth2AuthorizationCodeFlow>

#include <map>

class Osdu : public QObject {
  Q_OBJECT
public:
  Osdu(QObject *parent, const QString &server, const QString &dataParitionId,
       const QString &authority, const QString &scopes, const QString &clientId,
       QCommandLineParser *parser);
  QNetworkReply *makeRequest(const std::map<QString, QString> &parameters,
                             const QString &server,
                             const QString &dataPartitionId,
                             const QString &token);

  QNetworkReply *makeDownloadRequest(const QString &server,
                                     const QString &dataPartitionId,
                                     const QString &id, const QString &token);

  virtual ~Osdu(){};

public slots:
  void run();
  void parseWells(QNetworkReply *reply);
  void parseWellTrajectory(QNetworkReply *reply);
  void saveFile(QNetworkReply *reply);

signals:
  void finished();

private:
  void doIt(const QString &server, const QString &dataPartitionId,
            const QString &token);
  void addStandardHeader(QNetworkRequest &networkRequest, const QString &token,
                         const QString &dataPartitionId);

  static QString generateRandomString(int length = 20);
  static QString constructSearchUrl(const QString &server);
  static QString constructDownloadUrl(const QString &server,
                                      const QString &fileId);
  static QString constructAuthUrl(const QString &authority);
  static QString constructTokenUrl(const QString &authority);

  QOAuth2AuthorizationCodeFlow *osdu;
  QNetworkAccessManager *m_networkAccessManager;
  QCommandLineParser *m_parser;
};
