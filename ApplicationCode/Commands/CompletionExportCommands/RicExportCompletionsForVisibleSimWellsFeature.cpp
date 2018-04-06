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

#include "RicExportCompletionsForVisibleSimWellsFeature.h"

#include "RiaApplication.h"

#include "RicExportFeatureImpl.h"
#include "RicExportCompletionDataSettingsUi.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RimDialogData.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSimWellInViewCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicExportCompletionsForVisibleSimWellsFeature, "RicExportCompletionsForVisibleSimWellsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionsForVisibleSimWellsFeature::isCommandEnabled()
{
    std::vector<RimSimWellInView*> simWells = visibleSimWells();

    if (simWells.empty())
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForVisibleSimWellsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimSimWellInView*> simWells = visibleSimWells();
    CVF_ASSERT(!simWells.empty());

    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir    = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData();
    exportSettings->showForSimWells();

    if (!exportSettings->caseToApply())
    {
        std::vector<RimCase*> cases;
        app->project()->allCases(cases);
        for (auto c : cases)
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(c);
            if (eclipseCase != nullptr)
            {
                exportSettings->caseToApply = eclipseCase;
                break;
            }
        }
    }

    if (exportSettings->folder().isEmpty()) exportSettings->folder = defaultDir;

    caf::PdmUiPropertyViewDialog propertyDialog(Riu3DMainWindowTools::mainWindowWidget(), exportSettings, "Export Completion Data", "");
    RicExportFeatureImpl::configureForExport(&propertyDialog);

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("COMPLETIONS", exportSettings->folder);

        std::vector<RimWellPath*> wellPaths;
        RicWellPathExportCompletionDataFeatureImpl::exportCompletions(wellPaths, simWells, *exportSettings);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForVisibleSimWellsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Completion Data for Visible Simulation Wells");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSimWellInView*> RicExportCompletionsForVisibleSimWellsFeature::visibleSimWells()
{
    std::vector<RimSimWellInView*> simWells;
    
    {
        std::vector<RimSimWellInViewCollection*> simWellCollection;
        caf::SelectionManager::instance()->objectsByType(&simWellCollection);

        if (simWellCollection.empty())
        {
            std::vector<RimSimWellInView*> selectedSimWells;
            caf::SelectionManager::instance()->objectsByType(&selectedSimWells);

            if (!selectedSimWells.empty())
            {
                RimSimWellInViewCollection* parent = nullptr;
                selectedSimWells[0]->firstAncestorOrThisOfType(parent);
                if (parent)
                {
                    simWellCollection.push_back(parent);
                }
            }
        }

        for (auto coll : simWellCollection)
        {
            for (const auto& wellPath : coll->wells())
            {
                if (wellPath->showWell())
                {
                    simWells.push_back(wellPath);
                }
            }
        }
    }

    std::set<RimSimWellInView*> uniqueWellPaths(simWells.begin(), simWells.end());
    simWells.assign(uniqueWellPaths.begin(), uniqueWellPaths.end());
    
    return simWells;
}

