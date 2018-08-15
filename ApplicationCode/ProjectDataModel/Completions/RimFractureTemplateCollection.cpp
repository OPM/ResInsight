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

#include "RiaLogging.h"
#include "RiaApplication.h"

#include "RigStatisticsMath.h"

#include "RigEclipseCaseData.h"
#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEllipseFractureTemplate.h"
#include "RimFracture.h"
#include "RimFractureTemplate.h"
#include "RimProject.h"
#include "RimSimWellInViewCollection.h"
#include "RimStimPlanColors.h"
#include "RimStimPlanFractureTemplate.h"
#include "RimTools.h"
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
RimFractureTemplateCollection::RimFractureTemplateCollection()
{
    CAF_PDM_InitObject("Fracture Templates", ":/FractureTemplates16x16.png", "", "");

    CAF_PDM_InitField(&m_defaultUnitsForFracTemplates, "DefaultUnitForTemplates",
                      caf::AppEnum<RiaEclipseUnitTools::UnitSystem>(RiaEclipseUnitTools::UNITS_METRIC),
                      "Default unit system for fracture templates", "", "", "");

    CAF_PDM_InitFieldNoDefault(&m_fractureDefinitions, "FractureDefinitions", "", "", "", "");
    m_fractureDefinitions.uiCapability()->setUiHidden(true);

    CAF_PDM_InitField(&m_nextValidFractureTemplateId, "NextValidFractureTemplateId", 0, "", "", "", "");
    m_nextValidFractureTemplateId.uiCapability()->setUiHidden(true);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplateCollection::~RimFractureTemplateCollection()
{
    m_fractureDefinitions.deleteAllChildObjects();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RimFractureTemplateCollection::fractureTemplate(int id) const
{
    for (const auto& templ : m_fractureDefinitions)
    {
        if (templ->id() == id) return templ;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimFractureTemplate*> RimFractureTemplateCollection::fractureTemplates() const
{
    std::vector<RimFractureTemplate*> templates;
    for (auto& templ : m_fractureDefinitions)
    {
        templates.push_back(templ);
    }
    return templates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::addFractureTemplate(RimFractureTemplate* templ)
{
    templ->setId(nextFractureTemplateId());
    m_fractureDefinitions.push_back(templ);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RiaEclipseUnitTools::UnitSystemType RimFractureTemplateCollection::defaultUnitSystemType() const
{
    return m_defaultUnitsForFracTemplates;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::setDefaultUnitSystemBasedOnLoadedCases()
{
    RimProject* proj = RiaApplication::instance()->project();

    std::vector<RimCase*> rimCases;
    proj->allCases(rimCases);

    RiaEclipseUnitTools::UnitSystem commonUnitSystemForAllCases = RiaEclipseUnitTools::UNITS_UNKNOWN;

    for (const auto& c : rimCases)
    {
        auto eclipseCase = dynamic_cast<RimEclipseCase*>(c);
        if (eclipseCase && eclipseCase->eclipseCaseData())
        {
            if (commonUnitSystemForAllCases == RiaEclipseUnitTools::UNITS_UNKNOWN)
            {
                commonUnitSystemForAllCases = eclipseCase->eclipseCaseData()->unitsType();
            }
            else if (commonUnitSystemForAllCases != eclipseCase->eclipseCaseData()->unitsType())
            {
                commonUnitSystemForAllCases = RiaEclipseUnitTools::UNITS_UNKNOWN;
                break;
            }
        }
    }

    if (commonUnitSystemForAllCases != RiaEclipseUnitTools::UNITS_UNKNOWN)
    {
        m_defaultUnitsForFracTemplates = commonUnitSystemForAllCases;
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RimFractureTemplateCollection::firstFractureOfUnit(RiaEclipseUnitTools::UnitSystem unitSet) const
{
    for (RimFractureTemplate* f : m_fractureDefinitions())
    {
        if (f->fractureTemplateUnit() == unitSet)
        {
            return f;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<std::pair<QString, QString> > RimFractureTemplateCollection::resultNamesAndUnits() const
{
    std::set<std::pair<QString, QString> > nameSet;

    for (const RimFractureTemplate* f : m_fractureDefinitions())
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

    for (const RimFractureTemplate* f : m_fractureDefinitions())
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
void RimFractureTemplateCollection::createAndAssignTemplateCopyForNonMatchingUnit()
{
    // If a fracture has different unit than the associated template, create a copy of template in correct unit

    std::vector<RimFractureTemplate*> templatesToBeAdded;

    for (RimFractureTemplate* fractureTemplate : m_fractureDefinitions())
    {
        if (fractureTemplate && fractureTemplate->fractureGrid())
        {
            RimFractureTemplate* templateWithMatchingUnit = nullptr;

            std::vector<RimFracture*> referringObjects;
            fractureTemplate->objectsWithReferringPtrFieldsOfType(referringObjects);

            for (auto fracture : referringObjects)
            {
                if (fracture && fracture->fractureUnit() != fractureTemplate->fractureTemplateUnit())
                {
                    if (!templateWithMatchingUnit)
                    {
                        templateWithMatchingUnit = dynamic_cast<RimFractureTemplate*>(fractureTemplate->xmlCapability()->copyByXmlSerialization(caf::PdmDefaultObjectFactory::instance()));

                        auto currentUnit = fractureTemplate->fractureTemplateUnit();
                        auto neededUnit = RiaEclipseUnitTools::UNITS_UNKNOWN;
                        if (currentUnit == RiaEclipseUnitTools::UNITS_METRIC)
                        {
                            neededUnit = RiaEclipseUnitTools::UNITS_FIELD;
                        }
                        else if (currentUnit == RiaEclipseUnitTools::UNITS_FIELD)
                        {
                            neededUnit = RiaEclipseUnitTools::UNITS_METRIC;
                        }

                        templateWithMatchingUnit->convertToUnitSystem(neededUnit);

                        QString name = templateWithMatchingUnit->name();
                        name += " (created to match fracture unit)";
                        templateWithMatchingUnit->setName(name);

                        templatesToBeAdded.push_back(templateWithMatchingUnit);
                    }

                    RiaLogging::warning("Detected fracture with different unit than fracture template. Creating copy of template "
                                        "with matching unit.");

                    CVF_ASSERT(templateWithMatchingUnit->fractureTemplateUnit() == fracture->fractureUnit());
                    fracture->setFractureTemplateNoUpdate(templateWithMatchingUnit);
                }
            }
        }
    }

    for (auto templateWithMatchingUnit : templatesToBeAdded)
    {
        m_fractureDefinitions.push_back(templateWithMatchingUnit);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RimFractureTemplateCollection::loadAndUpdateData()
{
    for (RimFractureTemplate* f : m_fractureDefinitions())
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
void RimFractureTemplateCollection::updateFilePathsFromProjectPath(const QString& newProjectPath, const QString& oldProjectPath)
{
    for (RimFractureTemplate* f : m_fractureDefinitions())
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
    // Assign template id if not already assigned
    for (auto& templ : m_fractureDefinitions)
    {
        if (templ->id() < 0) templ->setId(nextFractureTemplateId());
    }

    RimProject* proj = nullptr;
    this->firstAncestorOrThisOfType(proj);
    if (proj && proj->isProjectFileVersionEqualOrOlderThan("2018.1.0.103"))
    {
        bool setAllShowMeshToFalseOnAllEclipseViews = false;

        std::vector<RimWellPathFracture*> wellPathFractures;
        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
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
                eclipseView->fractureColors()->setShowStimPlanMesh(false);
                continue;
            }

            //Find all fractures in all simWells
            std::map<RimStimPlanFractureTemplate*, bool> stimPlanFractureTemplatesInView;

            std::vector<RimFracture*> fractures;
            if (eclipseView->wellCollection())
            {
                eclipseView->wellCollection()->descendantsIncludingThisOfType(fractures);
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
                eclipseView->fractureColors()->setShowStimPlanMesh(templateIt->first->showStimPlanMesh());
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
                    eclipseView->fractureColors()->setShowStimPlanMesh(false);
                }
                else
                {
                    eclipseView->fractureColors()->setShowStimPlanMesh(true);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
int RimFractureTemplateCollection::nextFractureTemplateId()
{
    int newId = m_nextValidFractureTemplateId;
    m_nextValidFractureTemplateId = m_nextValidFractureTemplateId + 1;

    return newId;
}
