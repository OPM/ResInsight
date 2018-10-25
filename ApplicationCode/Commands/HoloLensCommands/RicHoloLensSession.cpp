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

#include "RicHoloLensSession.h"
#include "RicHoloLensSessionManager.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "VdeVizDataExtractor.h"
#include "VdeFileExporter.h"
#include "VdePacketDirectory.h"
#include "VdeArrayDataPacket.h"

#include "cvfAssert.h"

#include <QDir>



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession::RicHoloLensSession()
:   m_isSessionValid(false),
    m_lastExtractionMetaDataSequenceNumber(-1),
    m_dbgEnableFileExport(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession::~RicHoloLensSession()
{
    destroySession();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession* RicHoloLensSession::createSession(const QString& serverUrl, const QString& sessionName)
{
    RicHoloLensSession* newSession = new RicHoloLensSession;

    newSession->m_restClient = new RicHoloLensRestClient(serverUrl, sessionName, newSession);
    newSession->m_restClient->createSession();

    // For now, leave this on!!!
    // We probably want to export this as a preference parameter
    newSession->m_dbgEnableFileExport = true;

    return newSession;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession* RicHoloLensSession::createDummyFileBackedSession()
{
    RicHoloLensSession* newSession = new RicHoloLensSession;

    newSession->m_isSessionValid = true;

    newSession->m_dbgEnableFileExport = true;

    return newSession;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::destroySession()
{
    if (m_restClient)
    {
        if (m_isSessionValid)
        {
            m_restClient->deleteSession();
        }

        m_restClient->clearResponseHandler();
        m_restClient->deleteLater();
        m_restClient = nullptr;
    }

    m_isSessionValid = false;

    m_lastExtractionMetaDataSequenceNumber = -1;
    m_lastExtractionAllReferencedPacketIdsArr.clear();
    m_packetDirectory.clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensSession::isSessionValid() const
{
    return m_isSessionValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::updateSessionDataFromView(const RimGridView& activeView)
{
    RiaLogging::info("HoloLens: Updating visualization data");

    QString modelMetaJsonStr;
    std::vector<int> allReferencedPacketIds;
    m_packetDirectory.clear();

    VdeVizDataExtractor extractor(activeView);
    extractor.extractViewContents(&modelMetaJsonStr, &allReferencedPacketIds, &m_packetDirectory);

    m_lastExtractionMetaDataSequenceNumber++;
    m_lastExtractionAllReferencedPacketIdsArr = allReferencedPacketIds;

    if (m_restClient)
    {
        RiaLogging::info(QString("HoloLens: Sending updated meta data to sharing server (sequenceNumber=%1)").arg(m_lastExtractionMetaDataSequenceNumber));
        m_restClient->sendMetaData(m_lastExtractionMetaDataSequenceNumber, modelMetaJsonStr);
    }

    // Debug export to file
    if (m_dbgEnableFileExport)
    {
        const QString folderName = RiaApplication::instance()->preferences()->holoLensExportFolder();
        if (folderName.isEmpty())
        {
            RiaLogging::warning("HoloLens: Debug export to file enabled, but no export folder has been set");
            return;
        }

        const QDir outputDir(folderName);
        const QString absOutputFolder = outputDir.absolutePath();

        if (!outputDir.mkpath("."))
        {
            RiaLogging::error(QString("HoloLens: Could not create debug file export folder: %1").arg(absOutputFolder));
            return;
        }

        RiaLogging::info(QString("HoloLens: Doing debug export of data to folder: %1").arg(absOutputFolder));
        VdeFileExporter fileExporter(absOutputFolder);
        if (!fileExporter.exportToFile(modelMetaJsonStr, m_packetDirectory, allReferencedPacketIds))
        {
            RiaLogging::error("HoloLens: Error exporting debug data to folder");
        }

        RiaLogging::info("HoloLens: Done exporting debug data");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::handleSuccessfulCreateSession()
{
    RiaLogging::info("HoloLens: Session successfully created");
    m_isSessionValid = true;

    // Slight hack here - reaching out to the manager to update GUI
    // We should really just be notifying the manager that our state has changed
    RicHoloLensSessionManager::refreshToolbarState();
}

//--------------------------------------------------------------------------------------------------
/// Handle the server response we receive after sending new meta data
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::handleSuccessfulSendMetaData(int metaDataSequenceNumber)
{
    RiaLogging::info(QString("HoloLens: Processing server response to meta data (sequenceNumber=%1)").arg(metaDataSequenceNumber));

    if (m_lastExtractionMetaDataSequenceNumber != metaDataSequenceNumber)
    {
        RiaLogging::warning(QString("HoloLens: Ignoring server response, the sequenceNumber(%1) has been superseded").arg(metaDataSequenceNumber));
        return;
    }

    if (m_lastExtractionAllReferencedPacketIdsArr.size() > 0)
    {
        QByteArray combinedPacketArr;
        if (!m_packetDirectory.getPacketsAsCombinedBuffer(m_lastExtractionAllReferencedPacketIdsArr, &combinedPacketArr))
        {
            RiaLogging::warning("HoloLens: Error gathering the requested packets, no data will be sent");
            return;
        }

        RiaLogging::info(QString("HoloLens: Sending new data to sharing server (%1 packets)").arg(m_lastExtractionAllReferencedPacketIdsArr.size()));

        m_restClient->sendBinaryData(combinedPacketArr);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::handleError(const QString& errMsg, const QString& url, const QString& serverData)
{
    QString fullMsg = "HoloLens communication error: " + errMsg;

    if (!serverData.isEmpty())
    {
        fullMsg += "\n    serverMsg: " + serverData;
    }

    fullMsg += "\n    url: " + url;

    RiaLogging::error(fullMsg);
}

