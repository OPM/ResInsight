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

#include "RicHoloLensRestClient.h"

#include "VdePacketDirectory.h"

#include <QString>
#include <QPointer>

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
    ~RicHoloLensSession();

    static RicHoloLensSession*  createSession(const QString& serverUrl, const QString& sessionName);
    static RicHoloLensSession*  createDummyFileBackedSession();
    void                        destroySession();

    bool                        isSessionValid() const;

    void                        updateSessionDataFromView(const RimGridView& activeView);

private:
    RicHoloLensSession();

    virtual void    handleSuccessfulCreateSession() override;
    virtual void    handleSuccessfulSendMetaData(int metaDataSequenceNumber) override;
    virtual void    handleError(const QString& errMsg, const QString& url, const QString& serverData) override;

private:
    bool                            m_isSessionValid;
    QPointer<RicHoloLensRestClient> m_restClient;

    int                             m_lastExtractionMetaDataSequenceNumber;
    std::vector<int>                m_lastExtractionAllReferencedPacketIdsArr;
    VdePacketDirectory              m_packetDirectory;

    bool                            m_dbgEnableFileExport;
};



