#include "RiaOsduOAuthHttpServerReplyHandler.h"

#include <QAbstractOAuth>
#include <QOAuthHttpServerReplyHandler>
#include <QString>
#include <QUrl>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaOsduOAuthHttpServerReplyHandler::RiaOsduOAuthHttpServerReplyHandler( quint16 port, QObject* parent )
    : QOAuthHttpServerReplyHandler( port, parent )
    , m_port( port )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RiaOsduOAuthHttpServerReplyHandler::callback() const
{
    const QUrl url( QString::fromLatin1( "http://localhost:%1/" ).arg( m_port ) );
    return url.toString( QUrl::EncodeDelimiters );
}
