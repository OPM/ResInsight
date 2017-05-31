/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016      Statoil ASA
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

#include "RicChangeDataSourceFeatureUi.h"

#include "RimCase.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiaApplication.h"

CAF_PDM_SOURCE_INIT(RicChangeDataSourceFeatureUi, "ChangeDataSourceFeatureUi");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicChangeDataSourceFeatureUi::RicChangeDataSourceFeatureUi()
{
    CAF_PDM_InitObject("Change Data Source", "", "", "");

    CAF_PDM_InitFieldNoDefault(&wellPathToApply,  "CurveWellPath",  "Well Path", "", "", "");
    CAF_PDM_InitFieldNoDefault(&caseToApply,      "CurveCase",      "Case", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RicChangeDataSourceFeatureUi::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (fieldNeedingOptions == &caseToApply)
    {
        RimTools::caseOptionItems(&options);
    }
    else if (fieldNeedingOptions == &wellPathToApply)
    {
        RimTools::wellPathOptionItems(&options);
    }

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicChangeDataSourceFeatureUi::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    caf::PdmUiGroup* group = uiOrdering.addNewGroup("Apply the following for all selected curves");

    group->add(&caseToApply);
    group->add(&wellPathToApply);
}
