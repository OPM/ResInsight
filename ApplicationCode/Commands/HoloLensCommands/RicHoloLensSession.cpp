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

#include "RicHoloLensSession.h"
#include "RicHoloLensSessionObserver.h"

#include "RiaLogging.h"
#include "RiaPreferences.h"

#include "VdeArrayDataPacket.h"
#include "VdeFileExporter.h"
#include "VdePacketDirectory.h"
#include "VdeVizDataExtractor.h"

#include "cvfAssert.h"
#include "cvfTimer.h"

#include <QDir>

#include <algorithm>

//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession::RicHoloLensSession()
    : m_isSessionValid( false )
    , m_lastExtractionMetaDataSequenceNumber( -1 )
    , m_sessionObserver( nullptr )
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
RicHoloLensSession* RicHoloLensSession::createSession( const QString&              serverUrl,
                                                       const QString&              sessionName,
                                                       const QByteArray&           sessionPinCode,
                                                       RicHoloLensSessionObserver* sessionObserver )
{
    RicHoloLensSession* newSession = new RicHoloLensSession;

    newSession->m_restClient = new RicHoloLensRestClient( serverUrl, sessionName, newSession );

    if ( RiaApplication::instance()->preferences()->holoLensDisableCertificateVerification() )
    {
        RiaLogging::warning( "HoloLens: Disabling certificate verification for HTTPS connections" );
        newSession->m_restClient->dbgDisableCertificateVerification();
    }

    newSession->m_restClient->createSession( sessionPinCode );

    newSession->m_sessionObserver = sessionObserver;

    const QString dbgExportFolder = RiaApplication::instance()->preferences()->holoLensExportFolder();
    if ( !dbgExportFolder.isEmpty() )
    {
        newSession->m_dbgFileExportDestinationFolder = dbgExportFolder;
        RiaLogging::info( QString( "HoloLens: Debug file export will be written to folder: %1" ).arg( dbgExportFolder ) );
    }

    return newSession;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession* RicHoloLensSession::createDummyFileBackedSession()
{
    RicHoloLensSession* newSession = new RicHoloLensSession;

    newSession->m_isSessionValid = true;

    newSession->m_dbgFileExportDestinationFolder = RiaApplication::instance()->preferences()->holoLensExportFolder();

    return newSession;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::destroySession()
{
    if ( m_restClient )
    {
        if ( m_isSessionValid )
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
void RicHoloLensSession::updateSessionDataFromView( const RimGridView& activeView )
{
    RiaLogging::info( "HoloLens: Updating visualization data" );

    // Grab the current max ID as an easy way to detect if new IDs have been added for debugging purposes
    const int dbgMaxAssignedIdBeforeExtraction = m_cachingIdFactory.lastAssignedId();

    // Note that we pass the caching ID factory on the constructor which will try and detect data payloads that
    // are equal, and will then "recycle" the array IDs for these
    VdeVizDataExtractor extractor( activeView, &m_cachingIdFactory );

    QString          modelMetaJsonStr;
    std::vector<int> allReferencedPacketIds;
    extractor.extractViewContents( &modelMetaJsonStr, &allReferencedPacketIds, &m_packetDirectory );

    // Note!
    // The packet directory should now contain all the packets that are being actively referenced.
    // We now prune out any packets that are no longer being referenced. This means we do no caching of actual packet
    // data over time and that we assume that the server will ask for data packets/arrays right after having received
    // updated meta data
    m_packetDirectory.pruneUnreferencedPackets( allReferencedPacketIds );

    m_lastExtractionMetaDataSequenceNumber++;
    m_lastExtractionAllReferencedPacketIdsArr = allReferencedPacketIds;

    if ( m_restClient )
    {
        RiaLogging::info( QString( "HoloLens: Sending updated meta data to sharing server (sequenceNumber=%1)" )
                              .arg( m_lastExtractionMetaDataSequenceNumber ) );
        m_restClient->sendMetaData( m_lastExtractionMetaDataSequenceNumber, modelMetaJsonStr );
    }

    // Debug export to file
    if ( !m_dbgFileExportDestinationFolder.isEmpty() )
    {
        const QDir    outputDir( m_dbgFileExportDestinationFolder );
        const QString absOutputFolder = outputDir.absolutePath();

        if ( !outputDir.mkpath( "." ) )
        {
            RiaLogging::error( QString( "HoloLens: Could not create debug file export folder: %1" ).arg( absOutputFolder ) );
            return;
        }

        // For debugging, write only the new packets to file
        // Determine which packets are new by comparing the IDs to the max known ID before extraction
        std::vector<int> packetIdsToWrite;
        for ( int packetId : allReferencedPacketIds )
        {
            if ( packetId > dbgMaxAssignedIdBeforeExtraction )
            {
                packetIdsToWrite.push_back( packetId );
            }
        }

        // This will write all packets seen in this extraction to file
        // packetIdsToWrite = allReferencedPacketIds;

        RiaLogging::info( QString( "HoloLens: Doing debug export of data (%1 packets) to folder: %2" )
                              .arg( packetIdsToWrite.size() )
                              .arg( absOutputFolder ) );
        VdeFileExporter fileExporter( absOutputFolder );
        if ( !fileExporter.exportToFile( modelMetaJsonStr, m_packetDirectory, packetIdsToWrite ) )
        {
            RiaLogging::error( "HoloLens: Error exporting debug data to folder" );
        }

        RiaLogging::info( "HoloLens: Done exporting debug data" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::handleSuccessfulCreateSession()
{
    RiaLogging::info( "HoloLens: Session successfully created" );
    m_isSessionValid = true;

    notifyObserver( RicHoloLensSessionObserver::CreateSessionSucceeded );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::handleFailedCreateSession()
{
    RiaLogging::error( "HoloLens: Failed to create session" );
    m_isSessionValid = false;

    notifyObserver( RicHoloLensSessionObserver::CreateSessionFailed );
}

//--------------------------------------------------------------------------------------------------
/// Handle the server response we receive after sending new meta data
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::handleSuccessfulSendMetaData( int               metaDataSequenceNumber,
                                                       const QByteArray& jsonServerResponseString )
{
    cvf::Timer tim;

    RiaLogging::info(
        QString( "HoloLens: Processing server response (meta data sequenceNumber=%1)" ).arg( metaDataSequenceNumber ) );

    if ( m_lastExtractionMetaDataSequenceNumber != metaDataSequenceNumber )
    {
        RiaLogging::warning(
            QString( "HoloLens: Ignoring server response, the meta data sequenceNumber(%1) has been superseded" )
                .arg( metaDataSequenceNumber ) );
        return;
    }

    std::vector<int> arrayIdsToSend;

    // cvf::Trace::show("Raw JSON response from server: '%s'", jsonServerResponseString.data());
    QByteArray trimmedServerResponseString = jsonServerResponseString.trimmed();
    if ( trimmedServerResponseString.size() > 0 )
    {
        if ( !parseJsonIntegerArray( trimmedServerResponseString, &arrayIdsToSend ) )
        {
            RiaLogging::error(
                "HoloLens: Error parsing array server response with array Ids, no data will be sent to server" );
            return;
        }
    }
    else
    {
        // An empty server response means we should send all array referenced by the last sent meta data
        if ( m_lastExtractionAllReferencedPacketIdsArr.size() > 0 )
        {
            arrayIdsToSend = m_lastExtractionAllReferencedPacketIdsArr;
            RiaLogging::info( "HoloLens: Empty server response, sending all arrays referenced by last meta data" );
        }
    }

    if ( arrayIdsToSend.size() == 0 )
    {
        RiaLogging::info( "HoloLens: Nothing to do, no data requested by server" );
        return;
    }

    RiaLogging::info(
        QString( "HoloLens: Start sending data to server, %1 data arrays have been requested" ).arg( arrayIdsToSend.size() ) );

    size_t totalBytesSent     = 0;
    size_t totalNumArraysSent = 0;

    const bool sendAsIndividualPackets = false;

    // Sending data packets one by one
    if ( sendAsIndividualPackets )
    {
        for ( size_t i = 0; i < arrayIdsToSend.size(); i++ )
        {
            const int                 arrayId = arrayIdsToSend[i];
            const VdeArrayDataPacket* packet  = m_packetDirectory.lookupPacket( arrayId );
            if ( !packet )
            {
                RiaLogging::warning(
                    QString( "HoloLens: Could not get the requested data from cache, array id: %1 " ).arg( arrayId ) );
                continue;
            }

            QByteArray packetByteArr( packet->fullPacketRawPtr(), static_cast<int>( packet->fullPacketSize() ) );

            RiaLogging::info( QString( "HoloLens:     sending array id: %1, %2KB (%3 bytes)" )
                                  .arg( arrayId )
                                  .arg( packetByteArr.size() / 1024.0, 0, 'f', 2 )
                                  .arg( packetByteArr.size() ) );

            m_restClient->sendBinaryData( packetByteArr, "arrId" + QByteArray::number( arrayId ) );

            totalNumArraysSent++;
            totalBytesSent += packetByteArr.size();
        }
    }
    // Sending all requested arrays/packets in one combined packet
    else
    {
        QByteArray combinedPacketArr;
        if ( !m_packetDirectory.getPacketsAsCombinedBuffer( arrayIdsToSend, &combinedPacketArr ) )
        {
            RiaLogging::warning( "HoloLens: Error gathering the requested arrays, no data will be sent" );
            return;
        }

        totalNumArraysSent = arrayIdsToSend.size();
        totalBytesSent     = combinedPacketArr.size();

        RiaLogging::info( QString( "HoloLens: Sending data to server (%1 arrays combined), %2KB (%3 bytes)" )
                              .arg( totalNumArraysSent )
                              .arg( totalBytesSent / 1024.0, 0, 'f', 2 )
                              .arg( totalBytesSent ) );

        m_restClient->sendBinaryData( combinedPacketArr, "metaSeqNum" + QByteArray::number( metaDataSequenceNumber ) );
    }

    const double totalMb = totalBytesSent / ( 1024.0 * 1024.0 );
    RiaLogging::info( QString( "HoloLens: Finished sending data to server, %1 arrays, total %2MB (%3 bytes) in %4ms" )
                          .arg( totalNumArraysSent )
                          .arg( totalMb, 0, 'f', 2 )
                          .arg( totalBytesSent )
                          .arg( static_cast<int>( tim.time() * 1000 ) ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensSession::parseJsonIntegerArray( const QByteArray& jsonString, std::vector<int>* integerArr )
{
    // cvf::Trace::show("jsonString: '%s'", jsonString.data());

    const int openBraceIdx  = jsonString.indexOf( '[' );
    const int closeBraceIdx = jsonString.lastIndexOf( ']' );
    if ( openBraceIdx < 0 || closeBraceIdx < 0 )
    {
        RiaLogging::debug( "Error parsing JSON array, could not find opening or closing braces" );
        return false;
    }

    if ( closeBraceIdx <= openBraceIdx )
    {
        RiaLogging::debug( "Error parsing JSON array, wrong placement of braces" );
        return false;
    }

    QByteArray arrayContents = jsonString.mid( openBraceIdx + 1, closeBraceIdx - openBraceIdx - 1 ).trimmed();
    // cvf::Trace::show("arrayContents: '%s'", arrayContents.data());

    QList<QByteArray> stringList = arrayContents.split( ',' );
    for ( const QByteArray& entry : stringList )
    {
        bool      convertOk = false;
        const int intVal    = entry.toInt( &convertOk );
        if ( convertOk )
        {
            integerArr->push_back( intVal );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::handleError( const QString& errMsg, const QString& url, const QString& serverData )
{
    QString fullMsg = "HoloLens communication error: " + errMsg;

    if ( !serverData.isEmpty() )
    {
        fullMsg += "\n    serverMsg: " + serverData;
    }

    fullMsg += "\n    url: " + url;

    RiaLogging::error( fullMsg );

    // It is probably not correct to always consider an error a state change, but for now
    notifyObserver( RicHoloLensSessionObserver::GeneralError );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::notifyObserver( RicHoloLensSessionObserver::Notification notification )
{
    if ( m_sessionObserver )
    {
        m_sessionObserver->handleSessionNotification( this, notification );
    }
}
