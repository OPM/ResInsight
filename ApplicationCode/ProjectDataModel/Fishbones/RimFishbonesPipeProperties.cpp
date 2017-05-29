/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RimFishbonesPipeProperties.h"

#include <cstdlib>


CAF_PDM_SOURCE_INIT(RimFishbonesPipeProperties, "FishbonesPipeProperties");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesPipeProperties::RimFishbonesPipeProperties()
{
    CAF_PDM_InitObject("FishbonesPipeProperties", "", "", "");

    CAF_PDM_InitField(&m_lateralHoleRadius, "LateralHoleRadius", 12.0, "Hole Radius [mm]",   "", "", "");
    CAF_PDM_InitField(&m_skinFactor,        "SkinFactor",        1.0,  "Skin Factor [0..1]", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesPipeProperties::~RimFishbonesPipeProperties()
{

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFishbonesPipeProperties::defineUiOrdering(QString uiConfigName, caf::PdmUiOrdering & uiOrdering)
{
    uiOrdering.add(&m_lateralHoleRadius);
    uiOrdering.add(&m_skinFactor);
}
