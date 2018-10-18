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
#include "cafPdmUiFilePathEditor.h"
#include "cafPdmUiOrdering.h"
#include "cafPdmUiPushButtonEditor.h"
#include "cafPdmUiTextEditor.h"

CAF_PDM_SOURCE_INIT(RicHoloLensCreateSessionUi, "RicHoloLensCreateSessionUi");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RicHoloLensCreateSessionUi::RicHoloLensCreateSessionUi()
{
    CAF_PDM_InitObject("HoloLens Create Session", "", "", "");

    CAF_PDM_InitField(&m_createSession, "CreateSession", false, "", "", "", "");
    caf::PdmUiPushButtonEditor::configureEditorForField(&m_createSession);

    CAF_PDM_InitField(&m_sessionName, "SessionName", QString("DummySessionName"), "Session Name", "", "", "");
    CAF_PDM_InitField(&m_sessionPinCode, "SessionPinCode", QString("1234"), "Session Pin Code", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_serverSettings, "ServerSettings", "Server Settings", "", "", "");
    m_serverSettings = new RicHoloLensServerSettings;

    CAF_PDM_InitFieldNoDefault(&m_statusTextProxy, "StatusText", "Status Text", "", "", "");
    m_statusTextProxy.registerGetMethod(this, &RicHoloLensCreateSessionUi::getStatusText);
    m_statusTextProxy.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());
    m_statusTextProxy.uiCapability()->setUiLabelPosition(caf::PdmUiItemInfo::TOP);

    caf::PdmSettings::readFieldsFromApplicationStore(m_serverSettings);

    m_statusText = "Server Status Unknown";
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
void RicHoloLensCreateSessionUi::fieldChangedByUi(const caf::PdmFieldHandle* changedField,
                                                  const QVariant&            oldValue,
                                                  const QVariant&            newValue)
{
    if (changedField == &m_createSession)
    {
        if (m_createSession)
        {
            QString msg = "Created Session : " + m_sessionName;
            setStatusText(msg);

            RiaLogging::info(msg);
        }

        m_createSession = false;
    }
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
        group->add(&m_createSession);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateSessionUi::defineEditorAttribute(const caf::PdmFieldHandle* field,
                                                       QString                    uiConfigName,
                                                       caf::PdmUiEditorAttribute* attribute)
{
    if (field == &m_createSession)
    {
        caf::PdmUiPushButtonEditorAttribute* pbAttribute = dynamic_cast<caf::PdmUiPushButtonEditorAttribute*>(attribute);
        if (pbAttribute)
        {
            pbAttribute->m_buttonText = "Create Session";
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QString RicHoloLensCreateSessionUi::getStatusText() const
{
    return m_statusText;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensCreateSessionUi::setStatusText(const QString& statusText)
{
    m_statusText = statusText;
}
