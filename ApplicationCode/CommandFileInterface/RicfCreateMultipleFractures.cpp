/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicfCreateMultipleFractures.h"

#include "RicfCommandFileExecutor.h"

#include "FractureCommands/RicCreateMultipleFracturesFeature.h"
#include "FractureCommands/RicCreateMultipleFracturesOptionItemUi.h"
#include "FractureCommands/RicCreateMultipleFracturesUi.h"

#include "RimProject.h"
#include "RimDialogData.h"
#include "RimFractureTemplate.h"
#include "RimOilField.h"
#include "RimEclipseCase.h"
#include "RimEclipseCaseCollection.h"
#include "RimWellPath.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaWellNameComparer.h"

#include "cafCmdFeatureManager.h"


CAF_PDM_SOURCE_INIT(RicfCreateMultipleFractures, "createMultipleFractures");

template<>
void caf::AppEnum< MultipleFractures::Action >::setUp()
{
    addItem(MultipleFractures::APPEND_FRACTURES, "APPEND_FRACTURES", "Append Fractures");
    addItem(MultipleFractures::REPLACE_FRACTURES, "REPLACE_FRACTURES", "Replace Fractures");

    setDefault(MultipleFractures::NONE);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfCreateMultipleFractures::RicfCreateMultipleFractures()
{
    RICF_InitField(&m_caseId,               "caseId",               -1,                     "Case ID", "", "", "");
    RICF_InitField(&m_wellPathNames,        "wellPathNames",        std::vector<QString>(), "Well Path Names", "", "", "");
    RICF_InitField(&m_maxDistFromWellTd,    "maxDistFromWellTd",    100.0,                  "Max Distance From Well TD", "", "", "");
    RICF_InitField(&m_maxFracturesPerWell,  "maxFracturesPerWell",  100,                    "Max Fractures per Well", "", "", "");
    RICF_InitField(&m_templateId,           "templateId",           -1,                     "Template ID", "", "", "");
    RICF_InitField(&m_topLayer,             "topLayer",             -1,                     "Top Layer", "", "", "");
    RICF_InitField(&m_baseLayer,            "baseLayer",            -1,                     "Base Layer", "", "", "");
    RICF_InitField(&m_spacing,              "spacing",              300.0,                  "Spacing", "", "", "");
    RICF_InitField(&m_action,               "action", caf::AppEnum<MultipleFractures::Action>(MultipleFractures::NONE), "Action", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfCreateMultipleFractures::execute()
{
    RimProject* project = RiaApplication::instance()->project();
    RiuCreateMultipleFractionsUi* settings = project->dialogData()->multipleFractionsData();

    // Get case and fracture template
    auto gridCase = caseFromId(m_caseId);
    auto fractureTemplate = fractureTemplateFromId(m_templateId);
    auto wellPaths = this->wellPaths();

    if (gridCase && fractureTemplate && !wellPaths.empty() && validateArguments())
    {
        RicCreateMultipleFracturesOptionItemUi* options = new RicCreateMultipleFracturesOptionItemUi();
        options->setValues(m_topLayer, m_baseLayer, fractureTemplate, m_spacing);

        settings->clearWellPaths();
        for (auto wellPath : wellPaths)
        {
            settings->addWellPath(wellPath);
        }

        settings->setValues(gridCase, m_maxDistFromWellTd, m_maxFracturesPerWell);
        settings->clearOptions();
        settings->insertOptionItem(nullptr, options);

        caf::CmdFeatureManager* commandManager = caf::CmdFeatureManager::instance();
        auto feature = dynamic_cast<RicCreateMultipleFracturesFeature*>(commandManager->getCommandFeature("RicCreateMultipleFracturesFeature"));

        if (feature)
        {
            if (m_action == MultipleFractures::APPEND_FRACTURES)    feature->appendFractures();
            if (m_action == MultipleFractures::REPLACE_FRACTURES)   feature->replaceFractures();
        }
    }
}


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicfCreateMultipleFractures::validateArguments() const
{
    bool valid = 
        m_caseId >= 0 &&
        m_templateId >= 0 &&
        !(m_action == MultipleFractures::NONE);

    if (valid) return true;

    RiaLogging::error(QString("createMultipleFractures: Command argument validation failed"));
    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RicfCreateMultipleFractures::caseFromId(int caseId) const
{
    for (RimEclipseCase* c : RiaApplication::instance()->project()->activeOilField()->analysisModels->cases())
    {
        if (c->caseId() == caseId)  return c;
    }

    RiaLogging::error(QString("createMultipleFractures: Could not find case with ID %1").arg(caseId));
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFractureTemplate* RicfCreateMultipleFractures::fractureTemplateFromId(int templateId) const
{
    for (RimFractureTemplate* t : RiaApplication::instance()->project()->allFractureTemplates())
    {
        if (t->id() == templateId)  return t;
    }

    RiaLogging::error(QString("createMultipleFractures: Could not find fracture template with ID %1").arg(templateId));
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicfCreateMultipleFractures::wellPaths() const
{
    std::vector<RimWellPath*> wellPaths;
    auto allWellPaths = RiaApplication::instance()->project()->allWellPaths();

    if (!m_wellPathNames.v().empty())
    {
        std::set<QString> wellPathNameSet(m_wellPathNames.v().begin(), m_wellPathNames.v().end());

        for (auto wellPath : allWellPaths)
        {
            if (!RiaWellNameComparer::tryMatchNameInList(wellPath->name(), m_wellPathNames.v()).isEmpty())
                wellPaths.push_back(wellPath);
        }
    }
    else
    {
        wellPaths = allWellPaths;
    }

    if (wellPaths.empty() || wellPaths.size() < m_wellPathNames.v().size())
    {
        RiaLogging::error(QString("createMultipleFractures: One or more well paths was not found"));
        std::this_thread::sleep_for(std::chrono::seconds(2));
    }
    return wellPaths;
}

