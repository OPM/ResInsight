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

#include "RimFractureCollection.h"

#include "RimFracture.h"
#include "cafPdmObject.h"




CAF_PDM_SOURCE_INIT(RimFractureCollection, "FractureCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureCollection::RimFractureCollection(void)
{
    CAF_PDM_InitObject("Fracture Collection", "", "", "");

    CAF_PDM_InitField(&isActive, "Active", true, "Active", "", "", "");
    
    CAF_PDM_InitFieldNoDefault(&fractures, "Fractures", "", "", "", "");
    fractures.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureCollection::~RimFractureCollection()
{
    fractures.deleteAllChildObjects();

}
