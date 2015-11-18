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

#include "RimCrossSection.h"




namespace caf {

template<>
void caf::AppEnum< RimCrossSection::CrossSectionEnum >::setUp()
{
    addItem(RimCrossSection::CS_WELL_PATH,       "WELL_PATH",       "Well Path");
    addItem(RimCrossSection::CS_SIMULATION_WELL, "SIMULATION_WELL", "Simulation Well");
    addItem(RimCrossSection::CS_USER_DEFINED,    "USER_DEFINED",    "User defined");
    setDefault(RimCrossSection::CS_WELL_PATH);
}

} 


CAF_PDM_SOURCE_INIT(RimCrossSection, "CrossSection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCrossSection::RimCrossSection()
{
    CAF_PDM_InitObject("Cross Section", "", "", "");

    CAF_PDM_InitField(&name,        "UserDescription",  QString("Cross Section Name"), "Name", "", "", "");
    CAF_PDM_InitField(&isActive,    "Active",           true, "Active", "", "", "");
    isActive.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&crossSectionType, "CrossSectionType", "Type", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimCrossSection::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimCrossSection::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (useOptionsOnly) (*useOptionsOnly) = true;

/*
    if (&gridIndex == fieldNeedingOptions)
    {
        for (int gIdx = 0; gIdx < parentContainer()->gridCount(); ++gIdx)
        {
            QString gridName;

            gridName += parentContainer()->gridName(gIdx);
            if (gIdx == 0)
            {
                if (gridName.isEmpty())
                    gridName += "Main Grid";
                else
                    gridName += " (Main Grid)";
            }

            caf::PdmOptionItemInfo item(gridName, (int)gIdx);
            options.push_back(item);
        }
    }
*/
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCrossSection::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimCrossSection::objectToggleField()
{
    return &isActive;
}
