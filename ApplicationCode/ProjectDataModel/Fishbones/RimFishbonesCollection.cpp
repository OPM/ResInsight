/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RimFishbonesCollection.h"

#include "RifWellPathImporter.h"

#include "RigWellPath.h"

#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesMultipleSubs.h"


CAF_PDM_SOURCE_INIT(RimFishbonesCollection, "FishbonesCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection::RimFishbonesCollection()
{
    CAF_PDM_InitObject("Fishbones", ":/Folder.png", "", "");

    m_name.uiCapability()->setUiHidden(true);
    m_name = "Fishbones";

    CAF_PDM_InitFieldNoDefault(&fishbonesSubs, "FishbonesSubs", "fishbonesSubs", "", "", "");

    fishbonesSubs.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_wellPathCollection, "WellPathCollection", "Well Paths", "", "", "");
    m_wellPathCollection = new RimFishboneWellPathCollection;
    m_wellPathCollection.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishboneWellPathCollection* RimFishbonesCollection::wellPathCollection() const
{
    CVF_ASSERT(m_wellPathCollection);

    return m_wellPathCollection();
}

