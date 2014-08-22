/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA, Ceetron Solutions AS
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

#include "RimNoCommonAreaNncCollection.h"

CAF_PDM_SOURCE_INIT(RimNoCommonAreaNncCollection, "RimNoCommonAreaNncCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimNoCommonAreaNncCollection::RimNoCommonAreaNncCollection()
{
    CAF_PDM_InitObject("RimNoCommonAreaNncCollection", "", "", "");

    CAF_PDM_InitFieldNoDefault(&noCommonAreaNncs, "NoCommonAreaNncs", "NoCommonAreaNncs", "", "", "");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimNoCommonAreaNncCollection::~RimNoCommonAreaNncCollection()
{
    noCommonAreaNncs.deleteAllChildObjects();
}

