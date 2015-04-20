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

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"

CAF_PDM_SOURCE_INIT(RimGeoMechCase, "ResInsightGeoMechCase");
//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::RimGeoMechCase(void)
{
    CAF_PDM_InitObject("Geomechanical Case", ":/GeoMechCase48x48.png", "", "");

    CAF_PDM_InitField(&caseUserDescription, "CaseUserDescription",  QString(), "Case name", "", "" ,"");
    CAF_PDM_InitField(&caseFileName, "CaseFileName", QString(), "Case file name", "", "", "");
    caseFileName.setUiReadOnly(true);
    CAF_PDM_InitFieldNoDefault(&geoMechViews, "GeoMechViews", "",  "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechCase::~RimGeoMechCase(void)
{
    geoMechViews.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechView* RimGeoMechCase::createAndAddReservoirView()
{
    RimGeoMechView* gmv = new RimGeoMechView();
    size_t i = geoMechViews().size();
    gmv->name = QString("View %1").arg(i + 1);

    geoMechViews.push_back(gmv);
    return gmv;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimGeoMechCase::userDescriptionField()
{
    return &caseUserDescription;
}

