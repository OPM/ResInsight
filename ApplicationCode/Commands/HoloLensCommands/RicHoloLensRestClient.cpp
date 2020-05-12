/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicHoloLensRestClient.h"

#include "cvfTrace.h"

#include <QDateTime>
#include <QNetworkRequest>
#include <QSslConfiguration>

// For getting time stamps for round-trip timing
#if defined( __linux__ )
#include <ctime>
#include <sys/time.h>
#endif

#ifndef QT_NO_OPENSSL
// Uncomment to enable experimental SSL support
// The experimental support must be revised before shipping
#define EXPERIMENTAL_SSL_SUPPORT
#endif

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensRestClient::RicHoloLensRestClient( QString                         serverUrl,
                                              QString                         sessionName,
                                              RicHoloLensRestResponseHandler* responseHandler )
    : m_serverUrl( serverUrl )
    , m_sessionName( sessionName )
    , m_responseHandler( responseHandler )
    , m_dbgDisableCertificateVerification( false )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::clearResponseHandler()
{
    m_responseHandler = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::dbgDisableCertificateVerification()
{
    m_dbgDisableCertificateVerification = true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::createSession( const QByteArray& sessionPinCode )
{
    const QString url = m_serverUrl + "/sessions/create/" + m_sessionName;
    cvf::Trace::show( "createSession: POST on url: %s", url.toLatin1().constData() );

    QNetworkRequest request( url );
    // request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    request.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "application/x-www-form-urlencoded" ) );
    request.setRawHeader( "PinCode", sessionPinCode );

#ifdef EXPERIMENTAL_SSL_SUPPORT
    // NOTE !!!
    // Apparently something like this is currently needed in order to get SSL/HTTPS going
    // Still, can't quite figure it out since it appears to be sufficient to do this on the first request
    // This will have to be investigated further, SP 20180924
    QSslConfiguration sslConf = request.sslConfiguration();

    // Needed this one to be able to connect to sharing server
    sslConf.setProtocol( QSsl::AnyProtocol );

    if ( m_dbgDisableCertificateVerification )
    {
        sslConf.setPeerVerifyMode( QSslSocket::VerifyNone );
    }

    request.setSslConfiguration( sslConf );
#endif

    QNetworkReply* reply = m_accessManager.post( request, QByteArray() );
    connect( reply, SIGNAL( finished() ), SLOT( slotCreateSessionFinished() ) );

#ifdef EXPERIMENTAL_SSL_SUPPORT
    connect( reply, SIGNAL( sslErrors( const QList<QSslError>& ) ), SLOT( slotSslErrors( const QList<QSslError>& ) ) );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::slotCreateSessionFinished()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>( sender() );
    if ( !reply )
    {
        return;
    }

    if ( detectAndHandleErrorReply( "createSession", reply ) )
    {
        reply->deleteLater();

        m_responseHandler->handleFailedCreateSession();

        return;
    }

    const QByteArray serverData = reply->readAll();
    // cvf::Trace::show("  serverResponse: %s", serverData.constData());

    // Currently we get the bearer token back in the response wholesale
    // The format we get is typically: "Bearer <the token>"
    // For example: "Bearer eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9"
    // Presumably the format of the response will change, but for now just strip away the starting "Bearer " string and
    // consider the rest as the actual token
    m_bearerToken = serverData;
    m_bearerToken.replace( "Bearer ", "" );

    reply->deleteLater();

    cvf::Trace::show( "createSession OK" );
    if ( m_responseHandler )
    {
        m_responseHandler->handleSuccessfulCreateSession();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::deleteSession()
{
    const QString url = m_serverUrl + "/sessions/delete/" + m_sessionName;
    cvf::Trace::show( "deleteSession: DELETE on url: %s", url.toLatin1().constData() );

    QNetworkRequest request( url );
    // request.setHeader(QNetworkRequest::ContentTypeHeader, QVariant("application/octet-stream"));
    request.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "application/x-www-form-urlencoded" ) );
    addBearerAuthenticationHeaderToRequest( &request );

    QNetworkReply* reply = m_accessManager.deleteResource( request );
    connect( reply, SIGNAL( finished() ), SLOT( slotDeleteSessionFinished() ) );

#ifdef EXPERIMENTAL_SSL_SUPPORT
    connect( reply, SIGNAL( sslErrors( const QList<QSslError>& ) ), SLOT( slotSslErrors( const QList<QSslError>& ) ) );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::slotDeleteSessionFinished()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>( sender() );
    if ( !reply )
    {
        return;
    }

    if ( detectAndHandleErrorReply( "deleteSession", reply ) )
    {
        reply->deleteLater();
        return;
    }

    reply->deleteLater();

    cvf::Trace::show( "deleteSession OK" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::sendMetaData( int metaDataSequenceNumber, const QString& jsonMetaDataString )
{
    const QString url = m_serverUrl + "/sessions/" + m_sessionName + "/metadata";
    cvf::Trace::show( "sendMetaData (metaDataSequenceNumber=%d): POST on url: %s",
                      metaDataSequenceNumber,
                      url.toLatin1().constData() );

    const qint64 sendStartTimeStamp_ms = getCurrentTimeStamp_ms();

    QNetworkRequest request( url );
    request.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "application/json" ) );
    addBearerAuthenticationHeaderToRequest( &request );

    const QByteArray jsonByteArr = jsonMetaDataString.toLatin1();

    QNetworkReply* reply = m_accessManager.post( request, jsonByteArr );
    reply->setProperty( "holo_metaDataSequenceNumber", QVariant( metaDataSequenceNumber ) );
    reply->setProperty( "holo_sendStartTimeStamp_ms", QVariant( sendStartTimeStamp_ms ) );

    connect( reply, SIGNAL( finished() ), SLOT( slotSendMetaDataFinished() ) );

#ifdef EXPERIMENTAL_SSL_SUPPORT
    connect( reply, SIGNAL( sslErrors( const QList<QSslError>& ) ), SLOT( slotSslErrors( const QList<QSslError>& ) ) );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::slotSendMetaDataFinished()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>( sender() );
    if ( !reply )
    {
        return;
    }

    if ( detectAndHandleErrorReply( "sendMetaData", reply ) )
    {
        reply->deleteLater();
        return;
    }

    int metaDataSequenceNumber = -1;
    {
        QVariant var = reply->property( "holo_metaDataSequenceNumber" );
        if ( var.type() == QVariant::Int )
        {
            metaDataSequenceNumber = var.toInt();
        }
    }

    double elapsedTime_s = -1;
    {
        QVariant var = reply->property( "holo_sendStartTimeStamp_ms" );
        if ( var.type() == QVariant::LongLong )
        {
            const qint64 startTimeStamp_ms = var.toLongLong();
            elapsedTime_s                  = ( getCurrentTimeStamp_ms() - startTimeStamp_ms ) / 1000.0;
        }
    }

    const QByteArray serverData = reply->readAll();
    // cvf::Trace::show("  serverResponse: %s", serverData.constData());

    reply->deleteLater();

    cvf::Trace::show( "sendMetaData (metaDataSequenceNumber=%d) OK, elapsedTime=%.2fs",
                      metaDataSequenceNumber,
                      elapsedTime_s );
    if ( m_responseHandler )
    {
        m_responseHandler->handleSuccessfulSendMetaData( metaDataSequenceNumber, serverData );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::sendBinaryData( const QByteArray& binaryDataArr, QByteArray dbgTagString )
{
    const QString url = m_serverUrl + "/sessions/" + m_sessionName + "/data";
    cvf::Trace::show( "sendBinaryData(%s): POST on url: %s", dbgTagString.constData(), url.toLatin1().constData() );

    const qint64 sendStartTimeStamp_ms = getCurrentTimeStamp_ms();

    QNetworkRequest request( url );
    request.setHeader( QNetworkRequest::ContentTypeHeader, QVariant( "application/octet-stream" ) );
    addBearerAuthenticationHeaderToRequest( &request );

    QNetworkReply* reply = m_accessManager.post( request, binaryDataArr );
    reply->setProperty( "holo_sendStartTimeStamp_ms", QVariant( sendStartTimeStamp_ms ) );
    reply->setProperty( "holo_dbgTagString", QVariant( dbgTagString ) );

    connect( reply, SIGNAL( finished() ), SLOT( slotSendBinaryDataFinished() ) );

    // Debugging!
    connect( reply, SIGNAL( uploadProgress( qint64, qint64 ) ), SLOT( slotDbgUploadProgress( qint64, qint64 ) ) );

#ifdef EXPERIMENTAL_SSL_SUPPORT
    connect( reply, SIGNAL( sslErrors( const QList<QSslError>& ) ), SLOT( slotSslErrors( const QList<QSslError>& ) ) );
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::slotSendBinaryDataFinished()
{
    QNetworkReply* reply = dynamic_cast<QNetworkReply*>( sender() );
    if ( !reply )
    {
        return;
    }

    if ( detectAndHandleErrorReply( "sendBinaryData", reply ) )
    {
        reply->deleteLater();
        return;
    }

    double elapsedTime_s = -1;
    {
        QVariant var = reply->property( "holo_sendStartTimeStamp_ms" );
        if ( var.type() == QVariant::LongLong )
        {
            const qint64 startTimeStamp_ms = var.toLongLong();
            elapsedTime_s                  = ( getCurrentTimeStamp_ms() - startTimeStamp_ms ) / 1000.0;
        }
    }

    QByteArray dbgTagString;
    {
        QVariant var = reply->property( "holo_dbgTagString" );
        if ( var.type() == QVariant::ByteArray )
        {
            dbgTagString = var.toByteArray();
        }
    }

    reply->deleteLater();

    cvf::Trace::show( "sendBinaryData(%s) OK, elapsedTime=%.2fs", dbgTagString.constData(), elapsedTime_s );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::slotDbgUploadProgress( qint64 bytesSent, qint64 bytesTotal )
{
    static int sl_lastPct = -1;
    int        pct        = 0;
    if ( bytesTotal > 0 )
    {
        pct = static_cast<int>( 100 * ( bytesSent / static_cast<double>( bytesTotal ) ) );
    }

    if ( pct % 10 == 0 && pct != sl_lastPct )
    {
        double         elapsedTime_s = -1;
        QByteArray     dbgTagString;
        QNetworkReply* reply = dynamic_cast<QNetworkReply*>( sender() );
        if ( reply )
        {
            {
                QVariant var = reply->property( "holo_sendStartTimeStamp_ms" );
                if ( var.type() == QVariant::LongLong )
                {
                    const qint64 startTimeStamp_ms = var.toLongLong();
                    elapsedTime_s                  = ( getCurrentTimeStamp_ms() - startTimeStamp_ms ) / 1000.0;
                }
            }
            {
                QVariant var = reply->property( "holo_dbgTagString" );
                if ( var.type() == QVariant::ByteArray )
                {
                    dbgTagString = var.toByteArray();
                }
            }
        }

        cvf::Trace::show( "Progress sendBinaryData(%s): %3d%%, %.2f/%.2fMB (elapsedTime=%.2fs)",
                          dbgTagString.constData(),
                          pct,
                          bytesSent / ( 1024.0 * 1024.0 ),
                          bytesTotal / ( 1024.0 * 1024.0 ),
                          elapsedTime_s );
        sl_lastPct = pct;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::slotSslErrors( const QList<QSslError>& errors )
{
#ifdef EXPERIMENTAL_SSL_SUPPORT
    cvf::Trace::show( "RicHoloLensRestClient::slotSslErrors()" );
    for ( int i = 0; i < errors.size(); i++ )
    {
        cvf::Trace::show( "  %s", errors[i].errorString().toLatin1().constData() );
    }
#endif
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensRestClient::addBearerAuthenticationHeaderToRequest( QNetworkRequest* request ) const
{
    CVF_ASSERT( request );

    request->setRawHeader( "Authorization", "Bearer " + m_bearerToken );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensRestClient::detectAndHandleErrorReply( QString operationName, QNetworkReply* reply )
{
    CVF_ASSERT( reply );

    const QNetworkReply::NetworkError nwErrCode = reply->error();
    const int httpStatusCode                    = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute ).toInt();
    if ( nwErrCode == QNetworkReply::NoError && httpStatusCode == 200 )
    {
        // No error detected
        return false;
    }

    QString mainErrMsg = operationName + " FAILED";
    if ( nwErrCode != QNetworkReply::NoError )
    {
        const QString nwErrCodeAsString = networkErrorCodeAsString( nwErrCode );
        const QString errText           = reply->errorString();
        mainErrMsg += QString( "  [nwErr='%1'(%2)  httpStatus=%3]:  %4" )
                          .arg( nwErrCodeAsString )
                          .arg( nwErrCode )
                          .arg( httpStatusCode )
                          .arg( errText );
    }
    else
    {
        mainErrMsg += QString( "  [httpStatus=%1]" ).arg( httpStatusCode );
    }

    cvf::Trace::show( mainErrMsg.toLatin1().constData() );

    reply->errorString();

    const QString url = reply->url().toString();
    cvf::Trace::show( "  url: %s", url.toLatin1().constData() );

    const QByteArray serverData = reply->readAll();
    cvf::Trace::show( "  serverResponse: %s", serverData.constData() );

    if ( m_responseHandler )
    {
        m_responseHandler->handleError( mainErrMsg, url, serverData );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicHoloLensRestClient::networkErrorCodeAsString( QNetworkReply::NetworkError nwErr )
{
    switch ( nwErr )
    {
        case QNetworkReply::NoError:
            return "NoError";

        case QNetworkReply::ConnectionRefusedError:
            return "ConnectionRefusedError";
        case QNetworkReply::RemoteHostClosedError:
            return "RemoteHostClosedError";
        case QNetworkReply::HostNotFoundError:
            return "HostNotFoundError";
        case QNetworkReply::TimeoutError:
            return "TimeoutError";
        case QNetworkReply::OperationCanceledError:
            return "OperationCanceledError";
        case QNetworkReply::SslHandshakeFailedError:
            return "SslHandshakeFailedError";
        // case QNetworkReply::TemporaryNetworkFailureError:       return "TemporaryNetworkFailureError";
        case QNetworkReply::UnknownNetworkError:
            return "UnknownNetworkError";

        case QNetworkReply::ProxyConnectionRefusedError:
            return "ProxyConnectionRefusedError";
        case QNetworkReply::ProxyConnectionClosedError:
            return "ProxyConnectionClosedError";
        case QNetworkReply::ProxyNotFoundError:
            return "ProxyNotFoundError";
        case QNetworkReply::ProxyTimeoutError:
            return "ProxyTimeoutError";
        case QNetworkReply::ProxyAuthenticationRequiredError:
            return "ProxyAuthenticationRequiredError";
        case QNetworkReply::UnknownProxyError:
            return "UnknownProxyError";

        case QNetworkReply::ContentAccessDenied:
            return "ContentAccessDenied";
        case QNetworkReply::ContentOperationNotPermittedError:
            return "ContentOperationNotPermittedError";
        case QNetworkReply::ContentNotFoundError:
            return "ContentNotFoundError";
        case QNetworkReply::AuthenticationRequiredError:
            return "AuthenticationRequiredError";
        case QNetworkReply::ContentReSendError:
            return "ContentReSendError";
        case QNetworkReply::UnknownContentError:
            return "UnknownContentError";

        case QNetworkReply::ProtocolUnknownError:
            return "ProtocolUnknownError";
        case QNetworkReply::ProtocolInvalidOperationError:
            return "ProtocolInvalidOperationError";
        case QNetworkReply::ProtocolFailure:
            return "ProtocolFailure";
    };

    return "UnknownErrorCode";
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
qint64 RicHoloLensRestClient::getCurrentTimeStamp_ms()
{
    const qint64 timeStamp_ms = QDateTime::currentMSecsSinceEpoch();
    return timeStamp_ms;
}
