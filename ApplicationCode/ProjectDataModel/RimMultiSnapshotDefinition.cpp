/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RimMultiSnapshotDefinition.h"

#include "RiaApplication.h"
#include "RimCase.h"
#include "RimProject.h"
#include "RimView.h"

#include "cafPdmPointer.h"


CAF_PDM_SOURCE_INIT(RimMultiSnapshotDefinition, "MultiSnapshotDefinition");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMultiSnapshotDefinition::RimMultiSnapshotDefinition()
{
    //CAF_PDM_InitObject("MultiSnapshotDefinition", ":/Well.png", "", "");
    CAF_PDM_InitObject("MultiSnapshotDefinition", "", "", "");

    CAF_PDM_InitFieldNoDefault(&caseObject,     "Case",                 "Case", "", "", "");
    CAF_PDM_InitFieldNoDefault(&viewObject,     "View",                 "View", "", "", "");
    CAF_PDM_InitField(&timeStepStart,           "TimeStepStart", 0,     "Timestep Start", "", "", "");
    CAF_PDM_InitField(&timeStepEnd,             "TimeStepEnd", 0,       "Timestep End", "", "", "");
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimMultiSnapshotDefinition::~RimMultiSnapshotDefinition()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimMultiSnapshotDefinition::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    RimProject* proj = RiaApplication::instance()->project();

    if (fieldNeedingOptions == &caseObject && proj)
    {
        std::vector<RimCase*> cases;
        proj->allCases(cases);

        for (RimCase* c : cases)
        {
            options.push_back(caf::PdmOptionItemInfo(c->caseUserDescription(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(c))));
        }

        options.push_back(caf::PdmOptionItemInfo("All", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(nullptr))));
    }
    else if (fieldNeedingOptions == &viewObject)
    {
        if (caseObject())
        {
            std::vector<RimView*> views = caseObject()->views();
            for (RimView* view : views)
            {
                options.push_back(caf::PdmOptionItemInfo(view->name(), QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(view))));
            }
        }

        options.push_back(caf::PdmOptionItemInfo("All", QVariant::fromValue(caf::PdmPointer<caf::PdmObjectHandle>(nullptr))));
    }

    return options;
}

