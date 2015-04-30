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

#include "RimGeoMechResultSlot.h"
#include "RimGeoMechView.h"
#include "RimLegendConfig.h"
#include "RimDefines.h"




CAF_PDM_SOURCE_INIT(RimGeoMechResultSlot, "GeoMechResultSlot");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultSlot::RimGeoMechResultSlot(void)
{
  
    CAF_PDM_InitObject("Color Result", ":/CellResult.png", "", "");

    CAF_PDM_InitFieldNoDefault(&legendConfig, "LegendDefinition", "Legend Definition", "", "", "");
    this->legendConfig = new RimLegendConfig();

    CAF_PDM_InitField(&m_resultVariable, "ResultVariable", RimDefines::undefinedResultName(), "Variable", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimGeoMechResultSlot::~RimGeoMechResultSlot(void)
{
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QList<caf::PdmOptionItemInfo> RimGeoMechResultSlot::calculateValueOptions(const caf::PdmFieldHandle* fieldNeedingOptions, bool * useOptionsOnly)
{
    QList<caf::PdmOptionItemInfo> options;

    if (m_reservoirView)
    {
       // RimGeoMechCase* gmCase = m_reservoirView->geoMechCase();

    }

    if (&m_resultVariable == fieldNeedingOptions)
    {
        options.push_back(caf::PdmOptionItemInfo("Von Mises", QString("VonMises")) );
        options.push_back(caf::PdmOptionItemInfo("Sigma XX", QString("SIGXX")) );
        options.push_back(caf::PdmOptionItemInfo("Sigma YY", QString("SIGYY")) );

    }
    return options;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimGeoMechResultSlot::setReservoirView(RimGeoMechView* ownerReservoirView)
{
    m_reservoirView = ownerReservoirView;
}
