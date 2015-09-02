/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicLinkVisibleViewsFeatureUi.h"

#include "cafPdmUiTextEditor.h"
#include "cafPdmObjectGroup.h"

#include "RimView.h"
#include "RimLinkedViews.h"

CAF_PDM_SOURCE_INIT(RicLinkVisibleViewsFeatureUi, "RicLinkVisibleViewsFeatureUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicLinkVisibleViewsFeatureUi::RicLinkVisibleViewsFeatureUi(void)
{
    CAF_PDM_InitObject("Link Visible Views Feature UI", ":/chain.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_allViewsAsText,     "VisibleViews",     "Visible Views", "", "", "");
    m_allViewsAsText.uiCapability()->setUiEditorTypeName(caf::PdmUiTextEditor::uiEditorTypeName());

    CAF_PDM_InitFieldNoDefault(&m_masterView, "MasterView", "Master View", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeatureUi::setViews(const std::vector<RimView*>& allViews)
{
    m_allViews = allViews;

    QString viewNames;
    for (int i = 0; i < m_allViews.size(); i++)
    {
        viewNames += RimLinkedViews::displayNameForView(m_allViews[i]);
        viewNames += "\n";
    }
    m_allViewsAsText = viewNames;

    if (allViews.size() > 0)
    {
        m_masterView = allViews[0];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimView* RicLinkVisibleViewsFeatureUi::masterView()
{
    return m_masterView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicLinkVisibleViewsFeatureUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    if (fieldNeedingOptions == &m_masterView)
    {
        for (int i = 0; i < m_allViews.size(); i++)
        {
            optionList.push_back(caf::PdmOptionItemInfo(RimLinkedViews::displayNameForView(m_allViews[i]), 
                QVariant::fromValue(caf::PdmPointer<PdmObjectHandle>(m_allViews[i]))));
        }
    }

    return optionList;
}
