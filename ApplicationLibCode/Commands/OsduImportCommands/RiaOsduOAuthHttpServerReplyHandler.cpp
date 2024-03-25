#include "RiaOsduOAuthHttpServerReplyHandler.h"

#include <QAbstractOAuth>
// #include <QJsonDocument>
// #include <QJsonObject>
// #include <QNetworkAccessManager>
// #include <QNetworkReply>
// #include <QNetworkRequest>
// #include <QOAuth2AuthorizationCodeFlow>
// #include <QObject>

// #include <QDesktopServices>
#include <QOAuthHttpServerReplyHandler>
#include <QString>
#include <QUrl>
// #include <QUrlQuery>
// #include <qeventloop.h>

RiaOsduOAuthHttpServerReplyHandler::RiaOsduOAuthHttpServerReplyHandler( quint16 port, QObject* parent )
    : QOAuthHttpServerReplyHandler( port, parent )
    , m_port( port )
{
}

QString RiaOsduOAuthHttpServerReplyHandler::callback() const
{
    const QUrl url( QString::fromLatin1( "http://localhost:%1/" ).arg( m_port ) );
    return url.toString( QUrl::EncodeDelimiters );
}
