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

#include "RicExportCarfinForCompletionsFeature.h"

#include "RiaApplication.h"

#include "CompletionExportCommands/RicWellPathExportCompletionDataFeature.h"
#include "RicExportCarfinForCompletionsUi.h"

#include "RimDialogData.h"
#include "RimEclipseCase.h"
#include "RimWellPath.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "RiuPlotMainWindow.h"

#include <cafPdmUiPropertyViewDialog.h>
#include <cafSelectionManager.h>
#include <cafSelectionManagerTools.h>

#include <QAction>
#include <QFileInfo>


CAF_CMD_SOURCE_INIT(RicExportCarfinForCompletionsFeature, "RicExportCarfinForCompletionsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportCarfinForCompletionsUi* RicExportCarfinForCompletionsFeature::openDialog()
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    QString startPath = app->lastUsedDialogDirectory("WELL_PATH_EXPORT_DIR");
    if (startPath.isEmpty())
    {
        QFileInfo fi(proj->fileName());
        startPath = fi.absolutePath();
    }

    RicExportCarfinForCompletionsUi* featureUi = app->project()->dialogData()->exportCarfinForCompletionsData();
    if (featureUi->exportFolder().isEmpty())
    {
        featureUi->setExportFolder(startPath);
    }

    if (!featureUi->caseToApply())
    {
        std::vector<RimCase*> cases;
        app->project()->allCases(cases);
        for (auto c : cases)
        {
            RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(c);
            if (eclipseCase != nullptr)
            {
                featureUi->setCase(eclipseCase);
                break;
            }
        }
    }

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, featureUi, "Export Carfin", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(600, 230));

    if (propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty())
    {
        app->setLastUsedDialogDirectory("WELL_PATH_EXPORT_DIR", featureUi->exportFolder());
        return featureUi;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCarfinForCompletionsFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    return !wellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = visibleWellPaths();
    CVF_ASSERT(wellPaths.size() > 0);

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "Export Carfin";

    auto dialogData = openDialog();
    if (dialogData)
    {

    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCarfinForCompletionsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Carfin");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicExportCarfinForCompletionsFeature::visibleWellPaths()
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

        if (!wellPathCollections.empty())
        {
            for (auto wellPathCollection : wellPathCollections)
            {
                for (const auto& wellPath : wellPathCollection->wellPaths())
                {
                    if (wellPath->showWellPath())
                    {
                        wellPaths.push_back(wellPath);
                    }
                }
            }
        }
        else
        {
            // No well path or well path collection selected 

            auto allWellPaths = RiaApplication::instance()->project()->allWellPaths();
            for (const auto& w : allWellPaths)
            {
                if (w->showWellPath())
                {
                    wellPaths.push_back(w);
                }
            }

        }
    }

    std::set<RimWellPath*> uniqueWellPaths(wellPaths.begin(), wellPaths.end());
    wellPaths.assign(uniqueWellPaths.begin(), uniqueWellPaths.end());

    return wellPaths;
}
