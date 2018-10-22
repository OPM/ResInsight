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

#include "RiaLogging.h"

#include "cafCmdFeatureManager.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession::RicHoloLensSession()
    : m_isSessionValid(false)
    , m_isDummySession(false)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensSession* RicHoloLensSession::instance()
{
    static RicHoloLensSession theInstance;

    return &theInstance;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensSession::createSession(const QString& serverUrl, const QString& sessionName, const QString& sessionPinCode)
{
    if (isSessionValid())
    {
        RiaLogging::error("Terminate existing session before creating a new session");
        
        return false;
    }

    RiaLogging::info("url : " + serverUrl + " name : " + sessionName + " pinCode : " + sessionPinCode);

    m_isSessionValid = true;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensSession::createDummyFileBackedSession()
{
    if (isSessionValid())
    {
        RiaLogging::error("Terminate existing session before creating a new session");

        return false;
    }

    m_isDummySession = true;

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensSession::isSessionValid() const
{
    if (m_isDummySession) return true;

    return m_isSessionValid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::updateSessionDataFromView(RimGridView* activeView)
{
    RiaLogging::info("HoloLens : updateSessionDataFromView");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::terminateSession()
{
    if (!isSessionValid()) return;

    RiaLogging::info("Terminating HoloLens Session");

    m_isDummySession = false;
    m_isSessionValid = false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicHoloLensSession::refreshToolbarState()
{
    QStringList commandIds;

    commandIds << "RicHoloLensCreateSessionFeature";
    commandIds << "RicHoloLensExportToSharingServerFeature";
    commandIds << "RicHoloLensTerminateSessionFeature";

    caf::CmdFeatureManager::instance()->refreshEnabledState(commandIds);
}
