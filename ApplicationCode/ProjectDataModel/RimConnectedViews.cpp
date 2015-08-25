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

#include "RimConnectedViews.h"

#include "RimView.h"

#include "cafPdmFieldHandle.h"
#include "RimProject.h"
#include "RiaApplication.h"
#include "RimCase.h"

CAF_PDM_SOURCE_INIT(RimConnectedViews, "RimConnectedViews");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimConnectedViews::RimConnectedViews(void)
{
    CAF_PDM_InitObject("Connected Views", "", "", "");

    CAF_PDM_InitFieldNoDefault(&masterView, "MasterView", "Master View", "", "", "");
    CAF_PDM_InitFieldNoDefault(&slaveView, "SlaveView", "Slave View", "", "", "");
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimConnectedViews::~RimConnectedViews(void)
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimConnectedViews::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> optionList;

    std::vector<RimView*> views;
    allViews(views);

    for (size_t i = 0; i< views.size(); i++)
    {
        optionList.push_back(caf::PdmOptionItemInfo(views[i]->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(views[i]))));
    }

    if (optionList.size() > 0)
    {
        optionList.push_front(caf::PdmOptionItemInfo("None", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(NULL))));
    }

    return optionList;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimConnectedViews::allViews(std::vector<RimView*>& views)
{
    RimProject* proj = RiaApplication::instance()->project();

    if (proj)
    {
        std::vector<RimCase*> cases;
        proj->allCases(cases);
        for (size_t caseIdx = 0; caseIdx < cases.size(); caseIdx++)
        {
            RimCase* rimCase = cases[caseIdx];

            std::vector<RimView*> caseViews = rimCase->views();
            for (size_t viewIdx = 0; viewIdx < caseViews.size(); viewIdx++)
            {
                views.push_back(caseViews[viewIdx]);
            }
        }
    }
}

