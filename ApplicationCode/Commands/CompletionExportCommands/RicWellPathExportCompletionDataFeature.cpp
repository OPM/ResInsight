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

#include "RicWellPathExportCompletionDataFeature.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RiaApplication.h"

#include "RicExportFeatureImpl.h"

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

CAF_CMD_SOURCE_INIT(RicWellPathExportCompletionDataFeature, "RicWellPathExportCompletionDataFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::isCommandEnabled()
{
    std::vector<RimWellPath*>      wellPaths = selectedWellPaths();
    std::vector<RimSimWellInView*> simWells  = selectedSimWells();

    if (wellPaths.empty() && simWells.empty())
    {
        return false;
    }

    if (!wellPaths.empty() && !simWells.empty())
    {
        return false;
    }

    std::set<RimEclipseCase*> eclipseCases;
    for (auto simWell : simWells)
    {
        RimEclipseCase* eclipseCase;
        simWell->firstAncestorOrThisOfType(eclipseCase);
        eclipseCases.insert(eclipseCase);
    }
    if (eclipseCases.size() > 1)
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*>      wellPaths = selectedWellPaths();
    std::vector<RimSimWellInView*> simWells  = selectedSimWells();

    CVF_ASSERT(wellPaths.size() > 0 || simWells.size() > 0);

    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir    = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    bool                               onlyWellPathCollectionSelected = noWellPathsSelectedDirectly();
    RicExportCompletionDataSettingsUi* exportSettings =
        project->dialogData()->exportCompletionData(onlyWellPathCollectionSelected);

    if (wellPaths.empty())
    {
        exportSettings->showForSimWells();
    }
    else
    {
        exportSettings->showForWellPath();
    }

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

        RicWellPathExportCompletionDataFeatureImpl::exportCompletions(wellPaths, simWells, *exportSettings);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Completion Data");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathExportCompletionDataFeature::selectedWellPaths()
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType(&wellPaths);

    if (wellPaths.empty())
    {
        std::vector<RimWellPathCollection*> wellPathCollections;
        caf::SelectionManager::instance()->objectsByType(&wellPathCollections);

        for (auto wellPathCollection : wellPathCollections)
        {
            for (auto wellPath : wellPathCollection->wellPaths())
            {
                if (wellPath->showWellPath())
                {
                    wellPaths.push_back(wellPath);
                }
            }
        }
    }

    std::set<RimWellPath*> uniqueWellPaths(wellPaths.begin(), wellPaths.end());
    wellPaths.assign(uniqueWellPaths.begin(), uniqueWellPaths.end());
    return wellPaths;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportCompletionDataFeature::noWellPathsSelectedDirectly()
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType(&wellPaths);

    if (wellPaths.empty())
        return true;
    else
        return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimSimWellInView*> RicWellPathExportCompletionDataFeature::selectedSimWells()
{
    std::vector<RimSimWellInView*> simWells;
    caf::SelectionManager::instance()->objectsByType(&simWells);

    std::vector<RimSimWellInViewCollection*> simWellCollections;
    caf::SelectionManager::instance()->objectsByType(&simWellCollections);

    for (auto simWellCollection : simWellCollections)
    {
        for (auto simWell : simWellCollection->wells())
        {
            simWells.push_back(simWell);
        }
    }

    std::set<RimSimWellInView*> uniqueSimWells(simWells.begin(), simWells.end());
    simWells.assign(uniqueSimWells.begin(), uniqueSimWells.end());

    return simWells;
}
