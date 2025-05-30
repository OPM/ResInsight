#include "RiaOAuthHttpServerReplyHandler.h"

#include <QString>
#include <QUrl>
#include <QtNetworkAuth/QAbstractOAuth>
#include <QtNetworkAuth/QOAuthHttpServerReplyHandler>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOAuthHttpServerReplyHandler::RiaOAuthHttpServerReplyHandler( quint16 port, QObject* parent )
    : QOAuthHttpServerReplyHandler( port, parent )
    , m_port( port )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOAuthHttpServerReplyHandler::callback() const
{
    const QUrl url( QString::fromLatin1( "http://localhost:%1/" ).arg( m_port ) );
    return url.toString( QUrl::EncodeDelimiters );
}
