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

#pragma once

#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>

//==================================================================================================
//
//
//
//==================================================================================================
class RicHoloLensRestResponseHandler
{
public:
    virtual void handleSuccessfulCreateSession() = 0;
    virtual void handleFailedCreateSession()     = 0;

    virtual void handleSuccessfulSendMetaData( int metaDataSequenceNumber, const QByteArray& jsonServerResponseString ) = 0;

    virtual void handleError( const QString& errMsg, const QString& url, const QString& serverData ) = 0;
};

//==================================================================================================
//
//
//
//==================================================================================================
class RicHoloLensRestClient : public QObject
{
    Q_OBJECT

public:
    RicHoloLensRestClient( QString serverUrl, QString sessionName, RicHoloLensRestResponseHandler* responseHandler );

    void clearResponseHandler();

    void dbgDisableCertificateVerification();

    void createSession( const QByteArray& sessionPinCode );
    void deleteSession();
    void sendMetaData( int metaDataSequenceNumber, const QString& jsonMetaDataString );
    void sendBinaryData( const QByteArray& binaryDataArr, QByteArray dbgTagString );

private:
    void           addBearerAuthenticationHeaderToRequest( QNetworkRequest* request ) const;
    bool           detectAndHandleErrorReply( QString operationName, QNetworkReply* reply );
    static QString networkErrorCodeAsString( QNetworkReply::NetworkError nwErr );
    static qint64  getCurrentTimeStamp_ms();

private slots:
    void slotCreateSessionFinished();
    void slotDeleteSessionFinished();
    void slotSendMetaDataFinished();
    void slotSendBinaryDataFinished();
    void slotDbgUploadProgress( qint64 bytesSent, qint64 bytesTotal );

    void slotSslErrors( const QList<QSslError>& errors );

private:
    QNetworkAccessManager           m_accessManager;
    QString                         m_serverUrl;
    QString                         m_sessionName;
    RicHoloLensRestResponseHandler* m_responseHandler;

    bool m_dbgDisableCertificateVerification; // Debug option to disable certificate verification. Needed in order to
                                              // work with self-signed certifiactes

    QByteArray m_bearerToken;
};
