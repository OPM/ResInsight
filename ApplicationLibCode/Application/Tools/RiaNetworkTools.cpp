/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022 Equinor ASA
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

#include "RiaNetworkTools.h"

#include <QDesktopServices>
#include <QEventLoop>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QNetworkRequest>
#include <QStringList>
#include <QUrl>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaNetworkTools::openUrl( const QString& urlString )
{
    QDesktopServices::openUrl( QUrl( urlString ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaNetworkTools::createAndOpenUrlWithFallback( const QString& urlSubString )
{
    QStringList urls;

    QString urlStringWithPrefix = urlSubString.trimmed();
    if ( urlStringWithPrefix.isEmpty() ) return;
    if ( urlStringWithPrefix[0] != '/' ) urlStringWithPrefix = '/' + urlStringWithPrefix;

    urls += "https://resinsight.org" + urlStringWithPrefix;
    urls += "https://opm.github.io/ResInsight-UserDocumentation" + urlStringWithPrefix;

    openUrlWithFallback( urls );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaNetworkTools::openUrlWithFallback( const QStringList& urlList )
{
    for ( const auto& url : urlList )
    {
        if ( doesResourceExist( url ) )
        {
            QDesktopServices::openUrl( QUrl( url ) );
            return;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RiaNetworkTools::doesResourceExist( const QString& urlString )
{
    // Based on https://karanbalkar.com/posts/sending-http-request-using-qt5/

    // create custom temporary event loop on stack
    QEventLoop eventLoop;

    // "quit()" the event-loop, when the network request "finished()"
    QNetworkAccessManager mgr;
    QObject::connect( &mgr, SIGNAL( finished( QNetworkReply* ) ), &eventLoop, SLOT( quit() ) );

    // the HTTP request
    const QUrl      qurl( urlString );
    QNetworkRequest request( qurl );
    QNetworkReply*  reply = mgr.get( request );
    eventLoop.exec(); // blocks stack until "finished()" has been called

    auto statusCode = request.attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();

    auto error = reply->error();
    if ( error == QNetworkReply::NoError && statusCode == 200 )
    {
        delete reply;

        return true;
    }

    delete reply;

    return false;
}
