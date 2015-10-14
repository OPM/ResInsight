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

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimView.h"
#include "RimViewLinker.h"

CAF_PDM_SOURCE_INIT(RicLinkVisibleViewsFeatureUi, "RicLinkVisibleViewsFeatureUi");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicLinkVisibleViewsFeatureUi::RicLinkVisibleViewsFeatureUi(void)
{
    CAF_PDM_InitObject("Link Visible Views Feature UI", ":/chain.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_masterView, "MasterView", "Master View", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicLinkVisibleViewsFeatureUi::setViews(const std::vector<RimView*>& allViews)
{
    m_allViews = allViews;

    RimView* activeView = RiaApplication::instance()->activeReservoirView();

    // Set Active view as master view
    for (size_t i = 0; i < allViews.size(); i++)
    {
        if (activeView == allViews[i])
        {
            m_masterView = allViews[i];
        }
    }

    // Fallback to use first view if no active view is present
    if (!m_masterView && allViews.size() > 0)
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
        for (size_t i = 0; i < m_allViews.size(); i++)
        {
            RimCase* rimCase = NULL;
            m_allViews[i]->firstAnchestorOrThisOfType(rimCase);

            QIcon icon;
            if (rimCase)
            {
                icon = rimCase->uiCapability()->uiIcon();
            }


            optionList.push_back(caf::PdmOptionItemInfo(RimViewLinker::displayNameForView(m_allViews[i]), 
                QVariant::fromValue(caf::PdmPointer<PdmObjectHandle>(m_allViews[i])),
                false,
                icon));
        }
    }

    return optionList;
}
