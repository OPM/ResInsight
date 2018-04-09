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
void RicWellPathExportCompletionDataFeature::prepareExportSettingsAndExportCompletions(
    const QString&                        dialogTitle,
    const std::vector<RimWellPath*>&      wellPaths,
    const std::vector<RimSimWellInView*>& simWells)
{
    RiaApplication* app     = RiaApplication::instance();
    RimProject*     project = app->project();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir    = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RicExportCompletionDataSettingsUi* exportSettings = project->dialogData()->exportCompletionData();

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

    caf::PdmUiPropertyViewDialog propertyDialog(Riu3DMainWindowTools::mainWindowWidget(), exportSettings, dialogTitle, "");
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
bool RicWellPathExportCompletionDataFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();

    if (wellPaths.empty())
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
    std::vector<RimWellPath*> wellPaths = selectedWellPaths();
    CVF_ASSERT(!wellPaths.empty());

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "Export Completion Data for Selected Well Paths";

    RicWellPathExportCompletionDataFeature::prepareExportSettingsAndExportCompletions(dialogTitle, wellPaths, simWells);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathExportCompletionDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Completion Data for Selected Well Paths");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicWellPathExportCompletionDataFeature::selectedWellPaths()
{
    std::vector<RimWellPath*> wellPaths;
    caf::SelectionManager::instance()->objectsByType(&wellPaths);

    std::set<RimWellPath*> uniqueWellPaths(wellPaths.begin(), wellPaths.end());
    wellPaths.assign(uniqueWellPaths.begin(), uniqueWellPaths.end());

    return wellPaths;
}
