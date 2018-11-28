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

#include "RicHoloLensCreateSessionUi.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaOptionItemFactory.h"

#include "RicHoloLensServerSettings.h"

#include "cafPdmSettings.h"
#include "cafPdmUiOrdering.h"

CAF_PDM_SOURCE_INIT(RicHoloLensCreateSessionUi, "RicHoloLensCreateSessionUi");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensCreateSessionUi::RicHoloLensCreateSessionUi()
{
    CAF_PDM_InitObject("HoloLens Create Session", "", "", "");

    CAF_PDM_InitField(&m_sessionName, "SessionName", QString("DummySessionName"), "Session Name", "", "", "");
    CAF_PDM_InitField(&m_sessionPinCode, "SessionPinCode", QString("1234"), "Session Pin Code", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_serverSettings, "ServerSettings", "Server Settings", "", "", "");
    m_serverSettings = new RicHoloLensServerSettings;

    caf::PdmSettings::readFieldsFromApplicationStore(m_serverSettings);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensCreateSessionUi::~RicHoloLensCreateSessionUi()
{
    caf::PdmSettings::writeFieldsToApplicationStore(m_serverSettings);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicHoloLensCreateSessionUi::serverUrl() const
{
    CVF_ASSERT(m_serverSettings());

    return m_serverSettings->serverUrl();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicHoloLensCreateSessionUi::sessionName() const
{
    return m_sessionName;

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicHoloLensCreateSessionUi::sessionPinCode() const
{
    return m_sessionPinCode;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateSessionUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Server Configuration");

        m_serverSettings->uiOrdering(uiConfigName, *group);
    }

    {
        caf::PdmUiGroup* group = uiOrdering.addNewGroup("Create Session");

        group->add(&m_sessionName);
        group->add(&m_sessionPinCode);
    }
}
