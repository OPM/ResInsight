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
#include "Rim3dView.h"
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
void RicLinkVisibleViewsFeatureUi::setViews(const std::vector<Rim3dView*>& allViews)
{
    m_allViews = allViews;

    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();

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
Rim3dView* RicLinkVisibleViewsFeatureUi::masterView()
{
    return m_masterView;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicLinkVisibleViewsFeatureUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_masterView)
    {
        for (Rim3dView* v : m_allViews)
        {
            RimCase* rimCase = nullptr;
            v->firstAncestorOrThisOfType(rimCase);

            QIcon icon;
            if (rimCase)
            {
                icon = rimCase->uiCapability()->uiIcon();
            }

            options.push_back(caf::PdmOptionItemInfo(RimViewLinker::displayNameForView(v), v, false, icon));
        }
    }

    return options;
}
