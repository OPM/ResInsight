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

#include "RicHoloLensSessionManager.h"
#include "RicHoloLensSession.h"

#include "RiaLogging.h"

#include "cafCmdFeatureManager.h"



//==================================================================================================
//
//
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSessionManager::RicHoloLensSessionManager()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSessionManager* RicHoloLensSessionManager::instance()
{
    static RicHoloLensSessionManager theInstance;
    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensSessionManager::createSession(const QString& serverUrl, const QString& sessionName, const QString& /*sessionPinCode*/)
{
    if (m_session)
    {
        RiaLogging::error("Terminate existing session before creating a new session");
        return false;
    }

    RiaLogging::info(QString("Creating HoloLens session: '%1', server url: %2").arg(sessionName).arg(serverUrl));
    m_session = RicHoloLensSession::createSession(serverUrl, sessionName);

    refreshToolbarState();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensSessionManager::createDummyFileBackedSession()
{
    if (m_session)
    {
        RiaLogging::error("Terminate existing session before creating a new session");
        return false;
    }

    m_session = RicHoloLensSession::createDummyFileBackedSession();
    RiaLogging::info("Created dummy file-backed HoloLens session");

    refreshToolbarState();

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSessionManager::terminateSession()
{
    if (!m_session)
    {
        return;
    }

    RiaLogging::info("Terminating HoloLens session");
    m_session->destroySession();
    m_session->deleteLater();
    m_session = nullptr;

    refreshToolbarState();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicHoloLensSession* RicHoloLensSessionManager::session()
{
    return m_session;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHoloLensSessionManager::refreshToolbarState()
{
    QStringList commandIds;

    commandIds << "RicHoloLensCreateSessionFeature";
    commandIds << "RicHoloLensExportToSharingServerFeature";
    commandIds << "RicHoloLensTerminateSessionFeature";

    caf::CmdFeatureManager::instance()->refreshEnabledState(commandIds);
}

