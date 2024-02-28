#include "RiaFileDownloader.h"

#include <QCoreApplication>
#include <QDebug>
#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>

#include "RiaLogging.h"

RiaFileDownloader::RiaFileDownloader( QObject* parent )
    : QObject( parent )
{
}

void RiaFileDownloader::downloadFile( const QUrl& url, const QString& filePath )
{
    QNetworkAccessManager* manager = new QNetworkAccessManager( this );
    QNetworkRequest        request( url );

    RiaLogging::debug( "Downloading from: " + url.toString() );

    QNetworkReply* reply = manager->get( request );

    connect( reply,
             &QNetworkReply::finished,
             [=]()
             {
                 if ( reply->error() )
                 {
                     RiaLogging::error( "Download failed:" + reply->errorString() );
                     emit done();
                 }
                 else
                 {
                     QFile file( filePath );
                     if ( file.open( QIODevice::WriteOnly ) )
                     {
                         file.write( reply->readAll() );
                         file.close();
                         RiaLogging::info( "Download succeeded. File saved to " + filePath );
                         emit done();
                     }
                     else
                     {
                         RiaLogging::info( "Failed to save file to " + filePath );
                         emit done();
                     }
                 }
                 reply->deleteLater();
                 manager->deleteLater();
             } );
}
