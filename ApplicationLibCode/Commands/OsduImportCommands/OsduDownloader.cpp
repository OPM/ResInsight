#include "Downloader.h"

Downloader::Downloader(QObject *parent) : QObject(parent) {}

void Downloader::downloadFile(const QUrl &url, const QString &filePath) {
  QNetworkAccessManager *manager = new QNetworkAccessManager(this);
  QNetworkRequest request(url);

  qDebug() << "Downloading from:" << url;

  QNetworkReply *reply = manager->get(request);

  connect(reply, &QNetworkReply::finished, [=]() {
    if (reply->error()) {
      qDebug() << "Download failed:" << reply->errorString();
      emit done();
    } else {
      QFile file(filePath);
      if (file.open(QIODevice::WriteOnly)) {
        file.write(reply->readAll());
        file.close();
        qDebug() << "Download succeeded. File saved to" << filePath;
        emit done();
      } else {
        qDebug() << "Failed to save file to" << filePath;
        emit done();
      }
    }
    reply->deleteLater();
    manager->deleteLater();
  });
}
