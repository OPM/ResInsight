#include "osdu.hpp"
#include "Downloader.h"

#include <QAbstractOAuth>
#include <QJsonDocument>
#include <QJsonObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QOAuth2AuthorizationCodeFlow>
#include <QObject>

#include <QDesktopServices>
#include <QOAuthHttpServerReplyHandler>
#include <QString>
#include <QUrl>
#include <QUrlQuery>
#include <qeventloop.h>

class MyOAuthHttpServerReplyHandler : public QOAuthHttpServerReplyHandler {
public:
  MyOAuthHttpServerReplyHandler(quint16 port, QObject *parent)
      : QOAuthHttpServerReplyHandler(port, parent), m_port(port){};

  QString callback() const override {
    const QUrl url(QString::fromLatin1("http://localhost:%1/").arg(m_port));
    return url.toString(QUrl::EncodeDelimiters);
  };

private:
  quint16 m_port;
};

Osdu::Osdu(QObject *parent, const QString &server,
           const QString &dataPartitionId, const QString &authority,
           const QString &scopes, const QString &clientId,
           QCommandLineParser *parser)
    : QObject(parent) {

  m_networkAccessManager = new QNetworkAccessManager(this);
  this->osdu = new QOAuth2AuthorizationCodeFlow(this);
  m_parser = parser;

  if (m_parser->isSet("x")) {
    QString token = m_parser->value("x").trimmed();
    doIt(server, dataPartitionId, token);
  } else {
    int port = 35327;

    connect(this->osdu, &QOAuth2AuthorizationCodeFlow::authorizeWithBrowser,
            [=](QUrl url) {
              QUrlQuery query(url);
              url.setQuery(query);
              QDesktopServices::openUrl(url);
            });

    QString authUrl = constructAuthUrl(authority);
    this->osdu->setAuthorizationUrl(QUrl(authUrl));

    QString tokenUrl = constructTokenUrl(authority);
    this->osdu->setAccessTokenUrl(QUrl(tokenUrl));

    // App key
    this->osdu->setClientIdentifier(clientId);
    this->osdu->setScope(scopes);

    auto replyHandler = new MyOAuthHttpServerReplyHandler(port, this);
    this->osdu->setReplyHandler(replyHandler);

    connect(this->osdu, &QOAuth2AuthorizationCodeFlow::granted, [=]() {
      QString token = this->osdu->token();

      doIt(server, dataPartitionId, token);
    });
  }
}

void Osdu::doIt(const QString &server, const QString &dataPartitionId,
                const QString &token) {

  QString FIELD_KIND = "osdu:wks:master-data--Field:1.0.0";
  QString WELL_KIND = "osdu:wks:master-data--Well:1.1.0";
  QString WELLBORE_KIND = "osdu:wks:master-data--Wellbore:1.1.0";
  QString WELLBORE_TRAJECTORY_KIND =
      "osdu:wks:work-product-component--WellboreTrajectory:1.1.0";

  if (m_parser->isSet("l")) {
    printf("%s\n", token.toStdString().c_str());
    emit finished();
  } else if (m_parser->isSet("p")) {
    std::map<QString, QString> params;
    params["kind"] = FIELD_KIND;
    params["limit"] = "10000";
    params["query"] = "data.FieldName:IVAR*";
    makeRequest(params, server, dataPartitionId, token);

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(parseWells(QNetworkReply *)));
  } else if (m_parser->isSet("a")) {
    std::map<QString, QString> params;
    params["kind"] = WELL_KIND;
    params["limit"] = "10000";
    params["query"] = "data.FacilityName: \"NO*\"";
    makeRequest(params, server, dataPartitionId, token);

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(parseWells(QNetworkReply *)));
  } else if (m_parser->isSet("w")) {
    std::map<QString, QString> params;
    params["kind"] = WELL_KIND;
    params["limit"] = "10000";
    params["query"] = QString("nested(data.GeoContexts, (FieldID:\"%1\"))")
                          .arg(m_parser->value("w"));
    makeRequest(params, server, dataPartitionId, token);

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(parseWells(QNetworkReply *)));
  } else if (m_parser->isSet("b")) {

    std::map<QString, QString> params;
    params["kind"] = WELLBORE_KIND;
    params["limit"] = "10000";
    params["query"] = "data.WellID: \"" + m_parser->value("b") + "\"";
    makeRequest(params, server, dataPartitionId, token);

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(parseWells(QNetworkReply *)));
  } else if (m_parser->isSet("t")) {
    std::map<QString, QString> params;
    params["kind"] = WELLBORE_TRAJECTORY_KIND;
    params["limit"] = "10000";
    params["query"] = "data.WellboreID: \"" + m_parser->value("t") + "\"";
    makeRequest(params, server, dataPartitionId, token);

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(parseWellTrajectory(QNetworkReply *)));
  } else if (m_parser->isSet("f")) {
    QString fileId = m_parser->value("f");
    if (fileId.endsWith(":"))
      fileId.truncate(fileId.lastIndexOf(QChar(':')));
    makeDownloadRequest(server, dataPartitionId, fileId, token);

    connect(m_networkAccessManager, SIGNAL(finished(QNetworkReply *)), this,
            SLOT(saveFile(QNetworkReply *)));
  }
}

QString Osdu::constructSearchUrl(const QString &server) {
  return server + "/api/search/v2/query";
}

QString Osdu::constructDownloadUrl(const QString &server,
                                   const QString &fileId) {
  return server + "/api/file/v2/files/" + fileId + "/downloadURL";
}

QString Osdu::constructAuthUrl(const QString &authority) {
  return authority + "/oauth2/v2.0/authorize";
}

QString Osdu::constructTokenUrl(const QString &authority) {
  return authority + "/oauth2/v2.0/token";
}

QNetworkReply *Osdu::makeRequest(const std::map<QString, QString> &parameters,
                                 const QString &server,
                                 const QString &dataPartitionId,
                                 const QString &token) {
  QNetworkRequest m_networkRequest;
  m_networkRequest.setUrl(QUrl(constructSearchUrl(server)));

  addStandardHeader(m_networkRequest, token, dataPartitionId);

  QJsonObject obj;
  for (auto [key, value] : parameters) {
    obj.insert(key, value);
  }

  QJsonDocument doc(obj);
  QString strJson(doc.toJson(QJsonDocument::Compact));

  auto reply = m_networkAccessManager->post(m_networkRequest, strJson.toUtf8());
  return reply;
}

void Osdu::parseWells(QNetworkReply *reply) {
  qDebug() << "REQUEST FINISHED. Error? "
           << (reply->error() != QNetworkReply::NoError);

  QByteArray result = reply->readAll();

  reply->deleteLater();

  QJsonDocument doc = QJsonDocument::fromJson(result);
  // Extract the JSON object from the QJsonDocument
  QJsonObject jsonObj = doc.object();

  // Access "results" array from the JSON object
  QJsonArray resultsArray = jsonObj["results"].toArray();

  // Iterate through each element in the "results" array
  qDebug() << "Found " << resultsArray.size() << " items.";
  foreach (const QJsonValue &value, resultsArray) {
    QJsonObject resultObj = value.toObject();

    // Accessing specific fields from the result object
    QString id = resultObj["id"].toString();
    QString kind = resultObj["kind"].toString();

    qDebug() << "Id:" << id << " kind: " << kind;

    printf("%s\n", id.toStdString().c_str());
  }
  emit finished();
}

void Osdu::parseWellTrajectory(QNetworkReply *reply) {
  qDebug() << "REQUEST FINISHED. Error? "
           << (reply->error() != QNetworkReply::NoError);

  QByteArray result = reply->readAll();

  reply->deleteLater();

  QJsonDocument doc = QJsonDocument::fromJson(result);
  // Extract the JSON object from the QJsonDocument
  QJsonObject jsonObj = doc.object();

  QString formattedJsonString = doc.toJson(QJsonDocument::Indented);
  // qDebug() << formattedJsonString;

  // Access "results" array from the JSON object
  QJsonArray resultsArray = jsonObj["results"].toArray();

  // Iterate through each element in the "results" array
  //  qDebug() << "Found " << resultsArray.size() << " items.";
  foreach (const QJsonValue &value, resultsArray) {
    QJsonObject resultObj = value.toObject();

    // Accessing specific fields from the result object
    QJsonObject dataObj = resultObj["data"].toObject();

    //    QJsonArray id = resultObj["data"].toArray();
    QJsonArray dataSets = dataObj["Datasets"].toArray();
    foreach (const QJsonValue &dataSet, dataSets) {
      printf("%s\n", dataSet.toString().toStdString().c_str());
    }
  }

  emit finished();
}

void Osdu::saveFile(QNetworkReply *reply) {
  qDebug() << "REQUEST FINISHED. Error? "
           << (reply->error() != QNetworkReply::NoError);
  // if (reply->error() == QNetworkReply::NoError) {

  qDebug() << reply->errorString();
  QByteArray result = reply->readAll();

  reply->deleteLater();

  QEventLoop loop;

  QJsonDocument doc = QJsonDocument::fromJson(result);
  // Extract the JSON object from the QJsonDocument
  QJsonObject jsonObj = doc.object();

  QString signedUrl = jsonObj["SignedUrl"].toString();

  Downloader downloader;
  QUrl url(signedUrl);
  QString filePath = "/tmp/" + generateRandomString(30) + ".txt";
  downloader.downloadFile(url, filePath);

  QString formattedJsonString = doc.toJson(QJsonDocument::Indented);
  qDebug() << formattedJsonString;

  printf("%s => %s\n", signedUrl.toStdString().c_str(),
         filePath.toStdString().c_str());

  connect(&downloader, SIGNAL(done()), this, SIGNAL(finished()));

  loop.exec();
}

void Osdu::addStandardHeader(QNetworkRequest &networkRequest,
                             const QString &token,
                             const QString &dataPartitionId) {
  networkRequest.setHeader(QNetworkRequest::ContentTypeHeader,
                           "application/json");
  networkRequest.setRawHeader("Authorization", "Bearer " + token.toUtf8());
  networkRequest.setRawHeader(QByteArray("Data-Partition-Id"),
                              dataPartitionId.toUtf8());
}

QNetworkReply *Osdu::makeDownloadRequest(const QString &server,
                                         const QString &dataPartitionId,
                                         const QString &id,
                                         const QString &token) {
  QNetworkRequest m_networkRequest;

  QString url = constructDownloadUrl(server, id);

  m_networkRequest.setUrl(QUrl(url));

  addStandardHeader(m_networkRequest, token, dataPartitionId);

  auto reply = m_networkAccessManager->get(m_networkRequest);
  return reply;
}

QString Osdu::generateRandomString(int randomStringLength) {
  const QString possibleCharacters(
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789");
  QString randomString;
  for (int i = 0; i < randomStringLength; ++i) {
    quint32 value = QRandomGenerator::global()->generate();
    int index = value % possibleCharacters.length();
    QChar nextChar = possibleCharacters.at(index);
    randomString.append(nextChar);
  }
  return randomString;
}

void Osdu::run() { osdu->grant(); }
