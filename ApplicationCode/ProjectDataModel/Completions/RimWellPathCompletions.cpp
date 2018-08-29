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
#include "RimFishboneWellPathCollection.h"
#include "RimPerforationCollection.h"
#include "RimWellPathFractureCollection.h"

#include "cvfAssert.h"

#include "cafPdmUiTreeOrdering.h"


namespace caf {

    template<>
    void RimWellPathCompletions::WellTypeEnum::setUp()
    {
        addItem(RimWellPathCompletions::OIL, "OIL", "Oil");
        addItem(RimWellPathCompletions::GAS, "GAS", "Gas");
        addItem(RimWellPathCompletions::WATER, "WATER", "Water");
        addItem(RimWellPathCompletions::LIQUID, "LIQUID", "Liquid");

        setDefault(RimWellPathCompletions::OIL);
    }
}


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

    CAF_PDM_InitField(&m_wellNameForExport, "WellNameForExport", QString(), "Well Name for Completion Export", "", "", "");

    CAF_PDM_InitField(&m_wellGroupName, "WellGroupNameForExport", QString(), "Well Group Name for Completion Export", "", "", "");

    CAF_PDM_InitField(&m_referenceDepth, "ReferenceDepthForExport", QString(), "Reference Depth for Completion Export", "", "", "");

    CAF_PDM_InitField(&m_wellType, "WellTypeForExport", WellTypeEnum(), "Well Type for Completion Export", "", "", "");
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
void RimWellPathCompletions::setWellNameForExport(const QString& name)
{
    m_wellNameForExport = name;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellNameForExport() const
{
    return m_wellNameForExport();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellGroupName() const
{
    return m_wellGroupName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::referenceDepth() const
{
    return m_referenceDepth;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellTypeName() const
{
    return WellTypeEnum(m_wellType).uiText();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPathFractureCollection* RimWellPathCompletions::fractureCollection() const
{
    CVF_ASSERT(m_fractureCollection);

    return m_fractureCollection;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RimWellPathCompletions::hasCompletions() const
{
    if (!fractureCollection()->fractures().empty())
    {
        return true;
    }

    return !fishbonesCollection()->fishbonesSubs().empty() ||
           !fishbonesCollection()->wellPathCollection()->wellPaths().empty() ||
           !perforationCollection()->perforations().empty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::setUnitSystemSpecificDefaults()
{
    m_fishbonesCollection->setUnitSystemSpecificDefaults();
    m_fractureCollection->setUnitSystemSpecificDefaults();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::defineUiTreeOrdering(caf::PdmUiTreeOrdering& uiTreeOrdering, QString uiConfigName)
{ 
    uiTreeOrdering.skipRemainingChildren(true);

    if (!perforationCollection()->perforations().empty())
    {
        uiTreeOrdering.add(&m_perforationCollection);
    }

    if (!fishbonesCollection()->fishbonesSubs().empty() ||
        !fishbonesCollection()->wellPathCollection()->wellPaths().empty())
    {
        uiTreeOrdering.add(&m_fishbonesCollection);
    }

    if (!fractureCollection()->fractures().empty())
    {
        uiTreeOrdering.add(&m_fractureCollection);
    }
}
