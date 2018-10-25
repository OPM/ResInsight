/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>




//==================================================================================================
//
//
//
//==================================================================================================
class RicHoloLensRestResponseHandler
{
public:
    virtual void handleSuccessfulCreateSession() = 0;
    virtual void handleSuccessfulSendMetaData(int metaDataSequenceNumber) = 0;

    virtual void handleError(const QString& errMsg, const QString& url, const QString& serverData) = 0;
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
    RicHoloLensRestClient(QString serverUrl, QString sessionName, RicHoloLensRestResponseHandler* responseHandler);

    void    clearResponseHandler();

    void    createSession();
    void    deleteSession();
    void    sendMetaData(int metaDataSequenceNumber, const QString& jsonMetaDataString);
    void    sendBinaryData(const QByteArray& binaryDataArr);

private:
    bool            detectAndHandleErrorReply(QString operationName, QNetworkReply* reply);
    static QString  networkErrorCodeAsString(QNetworkReply::NetworkError nwErr);

private slots:
    void    slotCreateSessionFinished();
    void    slotDeleteSessionFinished();
    void    slotSendMetaDataFinished();
    void    slotSendBinaryDataFinished();

    void	slotSslErrors(const QList<QSslError>& errors);

private:
    QNetworkAccessManager               m_accessManager;
    QString                             m_serverUrl;
    QString                             m_sessionName;
    RicHoloLensRestResponseHandler*     m_responseHandler;
};
