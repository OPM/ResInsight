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
#include "RiaOptionItemFactory.h"

#include "Rim2dEclipseView.h"
#include "RimCase.h"
#include "RimGridView.h"
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
void RicLinkVisibleViewsFeatureUi::setViews(const std::vector<RimGridView*>& allViews)
{
    m_allViews = allViews;

    RimGridView* activeView = RiaApplication::instance()->activeGridView();

    std::vector<RimGridView*> masterCandidates = masterViewCandidates();

    // Set Active view as master view if the active view isn't a contour map.
    for (size_t i = 0; i < masterCandidates.size(); i++)
    {
        if (activeView == masterCandidates[i])
        {
            m_masterView = allViews[i];
        }
    }

    // Fallback to use first view if no active view is present
    if (!m_masterView && masterCandidates.size() > 0)
    {
        m_masterView = masterCandidates[0];
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGridView* RicLinkVisibleViewsFeatureUi::masterView()
{
    return m_masterView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimGridView*> RicLinkVisibleViewsFeatureUi::masterViewCandidates() const
{
    std::vector<RimGridView*> masterCandidates;
    // Set Active view as master view if the active view isn't a contour map.
    for (size_t i = 0; i < m_allViews.size(); i++)
    {
        Rim2dEclipseView* contourMap = dynamic_cast<Rim2dEclipseView*>(m_allViews[i]);
        if (contourMap == nullptr)
        {
            masterCandidates.push_back(m_allViews[i]);
        }
    }
    return masterCandidates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicLinkVisibleViewsFeatureUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, 
                                                                                  bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &m_masterView)
    {
        for (RimGridView* v : masterViewCandidates())
        {
            RiaOptionItemFactory::appendOptionItemFromViewNameAndCaseName(v, &options);
        }
    }

    return options;
}
