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

#include "RicHoloLensRestClient.h"
#include "RicHoloLensSessionObserver.h"

#include "VdeCachingHashedIdFactory.h"
#include "VdePacketDirectory.h"

#include <QPointer>
#include <QString>

#include <vector>

class RimGridView;

//==================================================================================================
//
//
//
//==================================================================================================
class RicHoloLensSession : public QObject, private RicHoloLensRestResponseHandler
{
public:
    ~RicHoloLensSession() override;

    static RicHoloLensSession* createSession( const QString&              serverUrl,
                                              const QString&              sessionName,
                                              const QByteArray&           sessionPinCode,
                                              RicHoloLensSessionObserver* sessionObserver );
    static RicHoloLensSession* createDummyFileBackedSession();
    void                       destroySession();

    bool isSessionValid() const;

    void updateSessionDataFromView( const RimGridView& activeView );

private:
    RicHoloLensSession();

    void handleSuccessfulCreateSession() override;
    void handleFailedCreateSession() override;
    void handleSuccessfulSendMetaData( int metaDataSequenceNumber, const QByteArray& jsonServerResponseString ) override;
    void handleError( const QString& errMsg, const QString& url, const QString& serverData ) override;

    static bool parseJsonIntegerArray( const QByteArray& jsonString, std::vector<int>* integerArr );

    void notifyObserver( RicHoloLensSessionObserver::Notification notification );

private:
    bool                            m_isSessionValid;
    QPointer<RicHoloLensRestClient> m_restClient;

    int                       m_lastExtractionMetaDataSequenceNumber;
    std::vector<int>          m_lastExtractionAllReferencedPacketIdsArr;
    VdeCachingHashedIdFactory m_cachingIdFactory;
    VdePacketDirectory        m_packetDirectory;

    RicHoloLensSessionObserver* m_sessionObserver;

    QString m_dbgFileExportDestinationFolder;
};
