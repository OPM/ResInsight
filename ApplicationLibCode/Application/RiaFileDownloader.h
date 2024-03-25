#include <QObject>
#include <QString>
#include <QUrl>

class RiaFileDownloader : public QObject
{
    Q_OBJECT
public:
    explicit RiaFileDownloader( QObject* parent = nullptr );

    void downloadFile( const QUrl& url, const QString& filePath );
signals:
    void done();
};
