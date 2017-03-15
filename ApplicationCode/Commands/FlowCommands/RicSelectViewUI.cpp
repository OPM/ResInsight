/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicSelectViewUI.h"

#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"


CAF_PDM_SOURCE_INIT(RicSelectViewUI, "RicSelectViewUI");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicSelectViewUI::RicSelectViewUI()
{
    CAF_PDM_InitObject("RicSelectViewUI", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_selectedView, "MasterView",           "Select view", "", "", "");
    CAF_PDM_InitField(&m_createNewView,         "CreateNewView", false, "Create New View", "", "", "");
    CAF_PDM_InitField(&m_newViewName,           "NewViewName",   QString("ShowContributingWells"), "New View Name", "", "", "");

    m_currentView = nullptr;
    m_currentCase = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectViewUI::setView(RimEclipseView* currentView)
{
    m_currentView = currentView;

    m_currentView->firstAncestorOrThisOfTypeAsserted(m_currentCase);

    m_selectedView = m_currentView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectViewUI::setCase(RimEclipseResultCase* currentCase)
{
    m_currentCase = currentCase;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicSelectViewUI::selectedView() const
{
    return m_selectedView();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSelectViewUI::createNewView() const
{
    return m_createNewView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSelectViewUI::newViewName() const
{
    return m_newViewName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicSelectViewUI::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_selectedView)
    {
        if (m_currentCase)
        {
            for (RimView* v : m_currentCase->views())
            {
                QIcon icon = v->uiCapability()->uiIcon();
                QString displayName = v->name;

                options.push_back(caf::PdmOptionItemInfo(displayName, v, false, icon));
            }
        }
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSelectViewUI::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    if (m_currentCase && m_currentCase->views().size() == 0)
    {
        m_createNewView = true;
    }

    if (m_createNewView)
    {
        m_newViewName.uiCapability()->setUiReadOnly(false);
        m_selectedView.uiCapability()->setUiReadOnly(true);
    }
    else
    {
        m_newViewName.uiCapability()->setUiReadOnly(true);
        m_selectedView.uiCapability()->setUiReadOnly(false);
    }
}

