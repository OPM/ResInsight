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

#include "RicfApplicationTools.h"
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

namespace caf
{
    template<>
    void AppEnum< MultipleFractures::Action >::setUp()
    {
        addItem(MultipleFractures::APPEND_FRACTURES, "APPEND_FRACTURES", "Append Fractures");
        addItem(MultipleFractures::REPLACE_FRACTURES, "REPLACE_FRACTURES", "Replace Fractures");

        setDefault(MultipleFractures::NONE);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicfCreateMultipleFractures::RicfCreateMultipleFractures()
{
    RICF_InitField(&m_caseId,               "caseId",               -1,                     "Case ID", "", "", "");
    RICF_InitField(&m_wellPathNames,        "wellPathNames",        std::vector<QString>(), "Well Path Names", "", "", "");
    RICF_InitField(&m_minDistFromWellTd,    "minDistFromWellTd",    100.0,                  "Min Distance From Well TD", "", "", "");
    RICF_InitField(&m_maxFracturesPerWell,  "maxFracturesPerWell",  100,                    "Max Fractures per Well", "", "", "");
    RICF_InitField(&m_templateId,           "templateId",           -1,                     "Template ID", "", "", "");
    RICF_InitField(&m_topLayer,             "topLayer",             -1,                     "Top Layer", "", "", "");
    RICF_InitField(&m_baseLayer,            "baseLayer",            -1,                     "Base Layer", "", "", "");
    RICF_InitField(&m_spacing,              "spacing",              300.0,                  "Spacing", "", "", "");
    RICF_InitField(&m_action,               "action", caf::AppEnum<MultipleFractures::Action>(MultipleFractures::APPEND_FRACTURES), "Action", "", "", "");

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicfCreateMultipleFractures::execute()
{
    using TOOLS = RicfApplicationTools;

    RimProject* project = RiaApplication::instance()->project();
    RiuCreateMultipleFractionsUi* settings = project->dialogData()->multipleFractionsData();

    // Get case and fracture template
    auto gridCase = TOOLS::caseFromId(m_caseId);
    auto fractureTemplate = fractureTemplateFromId(m_templateId);
    std::vector<RimWellPath*> wellPaths;

    // Find well paths
    {
        QStringList wellsNotFound;
        wellPaths = TOOLS::wellPathsFromNames(TOOLS::toQStringList(m_wellPathNames), &wellsNotFound);
        if (!wellsNotFound.empty())
        {
            RiaLogging::error(QString("createMultipleFractures: These well paths were not found: ") + wellsNotFound.join(", "));
        }
    }

    if (!gridCase)
    {
        RiaLogging::error(QString("createMultipleFractures: Could not find case with ID %1").arg(m_caseId));
    }

    if (!fractureTemplate)
    {
        RiaLogging::error(QString("createMultipleFractures: Could not find fracture template with ID %1").arg(m_templateId));
    }

    if (gridCase && fractureTemplate && !wellPaths.empty() && validateArguments())
    {
        RicCreateMultipleFracturesOptionItemUi* options = new RicCreateMultipleFracturesOptionItemUi();
        caf::CmdFeatureManager*                 commandManager = caf::CmdFeatureManager::instance();
        auto feature = dynamic_cast<RicCreateMultipleFracturesFeature*>(commandManager->getCommandFeature("RicCreateMultipleFracturesFeature"));

        // Default layers
        int topLayer = m_topLayer;
        int baseLayer = m_baseLayer;
        if (feature && (topLayer < 0 || baseLayer < 0))
        {
            auto ijkRange = feature->ijkRangeForGrid(gridCase);
            if (topLayer < 0) topLayer = static_cast<int>(ijkRange.first.z());
            if (baseLayer < 0) baseLayer = static_cast<int>(ijkRange.second.z());
        }
        options->setValues(topLayer, baseLayer, fractureTemplate, m_spacing);

        settings->clearWellPaths();
        for (auto wellPath : wellPaths)
        {
            settings->addWellPath(wellPath);
        }

        settings->setValues(gridCase, m_minDistFromWellTd, m_maxFracturesPerWell);
        settings->clearOptions();
        settings->insertOptionItem(nullptr, options);


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
        m_templateId >= 0;

    if (valid) return true;

    RiaLogging::error(QString("createMultipleFractures: Mandatory argument(s) missing"));
    return false;
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
