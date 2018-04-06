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

#include "RicExportCompletionsForVisibleWellPathsFeature.h"

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

CAF_CMD_SOURCE_INIT(RicExportCompletionsForVisibleWellPathsFeature, "RicExportCompletionsForVisibleWellPathsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionsForVisibleWellPathsFeature::isCommandEnabled()
{
    std::vector<RimWellPath*>      wellPaths = visibleWellPaths();

    if (wellPaths.empty())
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForVisibleWellPathsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*>      wellPaths = visibleWellPaths();
    CVF_ASSERT(wellPaths.size() > 0);
    
    std::vector<RimSimWellInView*> simWells;

    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir    = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData();
    exportSettings->showForWellPath();

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
void RicExportCompletionsForVisibleWellPathsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Completion Data for Visible Well Paths");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicExportCompletionsForVisibleWellPathsFeature::visibleWellPaths()
{
    std::vector<RimWellPath*> wellPaths;
    
    {
        std::vector<RimWellPathCollection*> wellPathCollections;
        caf::SelectionManager::instance()->objectsByType(&wellPathCollections);

        if (wellPathCollections.empty())
        {
            std::vector<RimWellPath*> selectedWellPaths;
            caf::SelectionManager::instance()->objectsByType(&selectedWellPaths);

            if (!selectedWellPaths.empty())
            {
                RimWellPathCollection* parent = nullptr;
                selectedWellPaths[0]->firstAncestorOrThisOfType(parent);
                if (parent)
                {
                    wellPathCollections.push_back(parent);
                }
            }
        }

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

