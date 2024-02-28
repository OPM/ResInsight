#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QUrl>

class Downloader : public QObject {
  Q_OBJECT
public:
  explicit Downloader(QObject *parent = nullptr);

  void downloadFile(const QUrl &url, const QString &filePath);
signals:
  void done();
};
