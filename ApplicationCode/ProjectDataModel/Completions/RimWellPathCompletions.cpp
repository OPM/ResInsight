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

#include "RimWellPathCompletions.h"

#include "RimFishbonesCollection.h"
#include "RimPerforationCollection.h"
#include "RimWellPathFractureCollection.h"

#include "cvfAssert.h"


CAF_PDM_SOURCE_INIT(RimWellPathCompletions, "WellPathCompletions");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathCompletions::RimWellPathCompletions()
{
    CAF_PDM_InitObject("Completions", ":/CompletionsSymbol16x16.png", "", "");

    CAF_PDM_InitFieldNoDefault(&m_perforationCollection, "Perforations", "Perforations", "", "", "");
    m_perforationCollection = new RimPerforationCollection;
    m_perforationCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_fishbonesCollection, "Fishbones", "Fishbones", "", "", "");
    m_fishbonesCollection = new RimFishbonesCollection;
    m_fishbonesCollection.uiCapability()->setUiHidden(true);

    CAF_PDM_InitFieldNoDefault(&m_fractureCollection, "Fractures", "Fractures", "", "", "");
    m_fractureCollection = new RimWellPathFractureCollection;
    m_fractureCollection.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RimWellPathCompletions::fishbonesCollection() const
{
    CVF_ASSERT(m_fishbonesCollection);

    return m_fishbonesCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimPerforationCollection* RimWellPathCompletions::perforationCollection() const
{
    CVF_ASSERT(m_perforationCollection);

    return m_perforationCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RimWellPathCompletions::fractureCollection() const
{
    CVF_ASSERT(m_fractureCollection);

    return m_fractureCollection;
}