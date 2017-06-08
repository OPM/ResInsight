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

#include "RimFractureTemplateCollection.h"

#include "RimFractureTemplate.h"
#include "RimStimPlanFractureTemplate.h"

#include "cafPdmObject.h"



CAF_PDM_SOURCE_INIT(RimFractureTemplateCollection, "FractureDefinitionCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::RimFractureTemplateCollection(void)
{
    CAF_PDM_InitObject("Fracture Templates", ":/FractureTemplates16x16.png", "", "");

    CAF_PDM_InitField(&defaultUnitsForFracTemplates, "defaultUnitForFracTemplates", caf::AppEnum<RimUnitSystem::UnitSystem>(RimUnitSystem::UNITS_METRIC), "Default unit system for fracture templates", "", "", "");
    CAF_PDM_InitFieldNoDefault(&fractureDefinitions, "FractureDefinitions", "", "", "", "");
    fractureDefinitions.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::~RimFractureTemplateCollection()
{
    fractureDefinitions.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString> > RimFractureTemplateCollection::stimPlanResultNamesAndUnits() const
{
    std::set<std::pair<QString, QString> > nameSet;

    for (const RimFractureTemplate* f : fractureDefinitions())
    {
        auto stimPlanFracture = dynamic_cast<const RimStimPlanFractureTemplate*>(f);
        if (stimPlanFracture)
        {
            std::vector<std::pair<QString, QString> > namesAndUnits = stimPlanFracture->getStimPlanPropertyNamesUnits();

            for (auto nameAndUnit : namesAndUnits)
            {
                nameSet.insert(nameAndUnit);
            }
        }
    }

    std::vector<std::pair<QString, QString>> names(nameSet.begin(), nameSet.end());

    return names;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<QString> RimFractureTemplateCollection::stimPlanResultNames() const
{
    std::vector<QString> names;

    for (auto nameAndUnit : stimPlanResultNamesAndUnits())
    {
        names.push_back(nameAndUnit.first);
    }

    return names;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::computeMinMax(const QString& resultName, const QString& unit, double* minValue, double* maxValue) const
{
    for (const RimFractureTemplate* f : fractureDefinitions())
    {
        auto stimPlanFracture = dynamic_cast<const RimStimPlanFractureTemplate*>(f);
        if (stimPlanFracture)
        {
            stimPlanFracture->computeMinMax(resultName, unit, minValue, maxValue);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::deleteFractureDefinitions()
{
    fractureDefinitions.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::loadAndUpdateData()
{
    for (RimFractureTemplate* f : fractureDefinitions())
    {
        RimStimPlanFractureTemplate* stimPlanFracture = dynamic_cast<RimStimPlanFractureTemplate*>(f);
        if (stimPlanFracture)
        {
            stimPlanFracture->loadDataAndUpdate();
        }
    }
}

