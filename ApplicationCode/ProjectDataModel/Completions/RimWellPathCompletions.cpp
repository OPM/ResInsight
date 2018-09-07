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

#include "RiaStdStringTools.h"

#include "RimFishbonesCollection.h"
#include "RimFishboneWellPathCollection.h"
#include "RimPerforationCollection.h"
#include "RimWellPathFractureCollection.h"

#include "cvfAssert.h"

#include "cafPdmUiTreeOrdering.h"

#include <cmath>

//--------------------------------------------------------------------------------------------------
/// Internal constants
//--------------------------------------------------------------------------------------------------
#define DOUBLE_INF  std::numeric_limits<double>::infinity()


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
    auto n = name;
    m_wellNameForExport = n.remove(' ');
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellNameForExport() const
{
    return formatStringForExport(m_wellNameForExport());
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellGroupNameForExport() const
{
    return formatStringForExport(m_wellGroupName, "1*");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::referenceDepthForExport() const
{
    std::string refDepth = m_referenceDepth.v().toStdString();
    if (RiaStdStringTools::isNumber(refDepth, '.'))
    {
        return m_referenceDepth.v();
    }
    return "1*";
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::wellTypeNameForExport() const
{
    switch (m_wellType.v())
    {
    case OIL:       return "OIL";
    case GAS:       return "GAS";
    case WATER:     return "WATER";
    case LIQUID:    return "LIQ";
    }
    return "";
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
    if (!fractureCollection()->allFractures().empty())
    {
        return true;
    }

    return !fishbonesCollection()->allFishbonesSubs().empty() ||
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

    if (!fishbonesCollection()->allFishbonesSubs().empty() ||
        !fishbonesCollection()->wellPathCollection()->wellPaths().empty())
    {
        uiTreeOrdering.add(&m_fishbonesCollection);
    }

    if (!fractureCollection()->allFractures().empty())
    {
        uiTreeOrdering.add(&m_fractureCollection);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimWellPathCompletions::fieldChangedByUi(const caf::PdmFieldHandle* changedField, const QVariant& oldValue, const QVariant& newValue)
{
    if (changedField == &m_referenceDepth)
    {
        if (!RiaStdStringTools::isNumber(m_referenceDepth.v().toStdString(), '.'))
        {
            if (!RiaStdStringTools::isNumber(m_referenceDepth.v().toStdString(), ','))
            {
                // Remove invalid input text
                m_referenceDepth = "";
            }
            else
            {
                // Wrong decimal sign entered, replace , by .
                auto text = m_referenceDepth.v();
                m_referenceDepth = text.replace(',', '.');
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RimWellPathCompletions::formatStringForExport(const QString& text, const QString& defaultValue) const
{
    if (text.isEmpty()) return defaultValue;
    if (text.contains(' ')) return QString("'%1'").arg(text);
    return text;
}
