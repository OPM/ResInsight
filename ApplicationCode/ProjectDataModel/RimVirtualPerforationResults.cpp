/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018 Statoil ASA
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

#include "RimVirtualPerforationResults.h"

#include "RimLegendConfig.h"



CAF_PDM_SOURCE_INIT(RimVirtualPerforationResults, "RimVirtualPerforationResults");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimVirtualPerforationResults::RimVirtualPerforationResults()
{
    CAF_PDM_InitObject("Virtual Perforation Results", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();
    legendConfig.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_showTensors, "ShowTensors", true, "", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimVirtualPerforationResults::~RimVirtualPerforationResults()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimVirtualPerforationResults::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimVirtualPerforationResults::objectToggleField()
{
    return &m_showTensors;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimVirtualPerforationResults::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool* useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;
    *useOptionsOnly = true;

    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimVirtualPerforationResults::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering& uiOrdering)
{
    uiOrdering.skipRemainingFields(true);
}

