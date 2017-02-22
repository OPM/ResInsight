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

#include "RimSimWellFractureCollection.h"

#include "RimSimWellFracture.h"
#include "cafPdmObject.h"




CAF_PDM_SOURCE_INIT(RimSimWellFractureCollection, "SimWellFractureCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFractureCollection::RimSimWellFractureCollection(void)
{
    CAF_PDM_InitObject("Fractures", ":/FractureSymbol16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&simwellFractures, "Fractures", "", "", "", "");
    simwellFractures.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimSimWellFractureCollection::~RimSimWellFractureCollection()
{
    simwellFractures.deleteAllChildObjects();

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimSimWellFractureCollection::deleteFractures()
{
    simwellFractures.deleteAllChildObjects();
}
