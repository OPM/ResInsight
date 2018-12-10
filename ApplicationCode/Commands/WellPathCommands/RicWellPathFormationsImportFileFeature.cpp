/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicWellPathFormationsImportFileFeature.h"

#include "RiaApplication.h"

#include "RimMainPlotCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicWellPathFormationsImportFileFeature, "RicWellPathFormationsImportFileFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellPathFormationsImportFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFormationsImportFileFeature::onActionTriggered(bool isChecked)
{
    // Open dialog box to select well path formations files
    RiaApplication* app = RiaApplication::instance();
    QString         defaultDir = app->lastUsedDialogDirectory("WELLPATHFORMATIONS_DIR");
    QStringList     wellPathFormationsFilePaths =
        QFileDialog::getOpenFileNames(Riu3DMainWindowTools::mainWindowWidget(), "Import Well Picks", defaultDir,
                                      "Well Picks (*.csv);;All Files (*.*)");

    if (wellPathFormationsFilePaths.size() < 1)
        return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("WELLPATHFORMATIONS_DIR", QFileInfo(wellPathFormationsFilePaths.last()).absolutePath());

    app->addWellPathFormationsToModel(wellPathFormationsFilePaths);

    RimProject* project = app->project();

    if (project)
    {
        project->scheduleCreateDisplayModelAndRedrawAllViews();
        if (project->mainPlotCollection())
        {
            project->mainPlotCollection->updatePlotsWithFormations();
        }

        RimOilField* oilField = project->activeOilField();

        if (!oilField) return;

        if (oilField->wellPathCollection->wellPaths().size() > 0)
        {
            RimWellPath* wellPath = oilField->wellPathCollection->mostRecentlyUpdatedWellPath();
            if (wellPath)
            {
                Riu3DMainWindowTools::selectAsCurrentItem(wellPath);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellPathFormationsImportFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Import Well Picks");
    actionToSetup->setIcon(QIcon(":/Formations16x16.png"));
}
