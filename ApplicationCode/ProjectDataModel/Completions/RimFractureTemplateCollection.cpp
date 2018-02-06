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

#include "RigStatisticsMath.h"

#include "RimCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "cafPdmObject.h"

#include <map>



CAF_PDM_SOURCE_INIT(RimFractureTemplateCollection, "FractureDefinitionCollection");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::RimFractureTemplateCollection(void)
{
    CAF_PDM_InitObject("Fracture Templates", ":/FractureTemplates16x16.png", "", "");

    CAF_PDM_InitField(&defaultUnitsForFracTemplates, "DefaultUnitForTemplates",
                      caf::AppEnum<RiaEclipseUnitTools::UnitSystem>(RiaEclipseUnitTools::UNITS_METRIC),
                      "Default unit system for fracture templates", "", "", "");

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
        std::vector<std::pair<QString, QString> > namesAndUnits = f->uiResultNamesWithUnit();

        for (const auto& nameAndUnit : namesAndUnits)
        {
            nameSet.insert(nameAndUnit);
        }
    }

    std::vector<std::pair<QString, QString>> names(nameSet.begin(), nameSet.end());

    return names;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::computeMinMax(const QString& uiResultName, const QString& unit, double* minValue,
                                                  double* maxValue, double* posClosestToZero, double* negClosestToZero) const
{
    MinMaxAccumulator minMaxAccumulator;
    PosNegAccumulator posNegAccumulator;

    for (const RimFractureTemplate* f : fractureDefinitions())
    {
        if (f)
        {
            f->appendDataToResultStatistics(uiResultName, unit, minMaxAccumulator, posNegAccumulator);
        }
    }

    if (*minValue) *minValue = minMaxAccumulator.min;
    if (*maxValue) *maxValue = minMaxAccumulator.max;
    if (*posClosestToZero) *posClosestToZero = posNegAccumulator.pos;
    if (*negClosestToZero) *negClosestToZero = posNegAccumulator.neg;
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::setDefaultConductivityResultIfEmpty()
{
    for (RimFractureTemplate* f : fractureDefinitions())
    {
        RimStimPlanFractureTemplate* stimPlanFracture = dynamic_cast<RimStimPlanFractureTemplate*>(f);
        if (stimPlanFracture)
        {
            stimPlanFracture->setDefaultConductivityResultIfEmpty();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    for (RimFractureTemplate* f : fractureDefinitions())
    {
        RimStimPlanFractureTemplate* stimPlanFracture = dynamic_cast<RimStimPlanFractureTemplate*>(f);
        if (stimPlanFracture)
        {
            stimPlanFracture->updateFilePathsFromProjectPath(newProjectPath, oldProjectPath);
        }

        RimEllipseFractureTemplate* ellipseFracture = dynamic_cast<RimEllipseFractureTemplate*>(f);
        if (ellipseFracture)
        {
            ellipseFracture->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::initAfterRead()
{
    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfType(proj);
    if (proj && proj->isProjectFileVersionEqualOrOlderThan("2018.1.0.103"))
    {
        bool setAllShowMeshToFalseOnAllEclipseViews = false;

        std::vector<RimWellPathFracture*> wellPathFractures;
        RimWellPathCollection* wellPathCollection = proj->activeOilField()->wellPathCollection();
        wellPathCollection->descendantsIncludingThisOfType(wellPathFractures);

        for (RimWellPathFracture* fracture : wellPathFractures)
        {
            RimStimPlanFractureTemplate* stimPlanFractureTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->fractureTemplate());
            if (stimPlanFractureTemplate)
            {
                if (stimPlanFractureTemplate->showStimPlanMesh() == false)
                {
                    setAllShowMeshToFalseOnAllEclipseViews = true;
                    break;
                }
            }
        }

        std::vector<RimEclipseView*> eclipseViews;
        
        std::vector<RimCase*> rimCases;
        proj->allCases(rimCases);

        for (RimCase* rimCase : rimCases)
        {
            for (Rim3dView* view : rimCase->views())
            {
                RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
                if (eclView)
                {
                    eclipseViews.push_back(eclView);
                }
            }
        }

        for (RimEclipseView* eclipseView : eclipseViews)
        {
            if (setAllShowMeshToFalseOnAllEclipseViews)
            {
                eclipseView->stimPlanColors->setShowStimPlanMesh(false);
                continue;
            }

            //Find all fractures in all simWells
            std::map<RimStimPlanFractureTemplate*, bool> stimPlanFractureTemplatesInView;

            std::vector<RimFracture*> fractures;
            if (eclipseView->wellCollection)
            {
                eclipseView->wellCollection->descendantsIncludingThisOfType(fractures);
            }
            if (fractures.empty()) continue;

            for (RimFracture* fracture : fractures)
            {
                RimStimPlanFractureTemplate* stimPlanFractureTemplate = dynamic_cast<RimStimPlanFractureTemplate*>(fracture->fractureTemplate());
                if (stimPlanFractureTemplate)
                {
                    stimPlanFractureTemplatesInView[stimPlanFractureTemplate];
                }
            }

            if (stimPlanFractureTemplatesInView.empty()) continue;

            auto templateIt = stimPlanFractureTemplatesInView.begin();

            if (stimPlanFractureTemplatesInView.size() == 1)
            {
                eclipseView->stimPlanColors->setShowStimPlanMesh(templateIt->first->showStimPlanMesh());
            }
            else
            {
                bool anySetShowStimPlanMeshIsSetToFalse = false;
                for (templateIt; templateIt != stimPlanFractureTemplatesInView.end(); templateIt++)
                {
                    if (templateIt->first->showStimPlanMesh() == false)
                    {
                        anySetShowStimPlanMeshIsSetToFalse = true;
                        break;
                    }
                }
                if (anySetShowStimPlanMeshIsSetToFalse)
                {
                    eclipseView->stimPlanColors->setShowStimPlanMesh(false);
                }
                else
                {
                    eclipseView->stimPlanColors->setShowStimPlanMesh(true);
                }
            }
        }
    }
}

