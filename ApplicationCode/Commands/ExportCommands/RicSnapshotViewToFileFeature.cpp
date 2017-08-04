/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicSnapshotViewToFileFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimViewWindow.h"
#include "RiuMainPlotWindow.h"

#include "RicSnapshotFilenameGenerator.h"

#include "cafUtils.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>
#include <QMessageBox>


CAF_CMD_SOURCE_INIT(RicSnapshotViewToFileFeature, "RicSnapshotViewToFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveSnapshotAs(const QString& fileName, RimViewWindow* viewWindow)
{
    if (viewWindow)
    {
        QImage image = viewWindow->snapshotWindowContent();
        if (!image.isNull())
        {
            if (image.save(fileName))
            {
                RiaLogging::info(QString("Exported snapshot image to %1").arg(fileName));
            }
            else
            {
                RiaLogging::error(QString("Error when trying to export snapshot image to %1").arg(fileName));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSnapshotViewToFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    // Get active view window before displaying the file selection dialog
    // If this is done after the file save dialog is displayed (and closed)
    // app->activeViewWindow() returns NULL on Linux
    RimViewWindow* viewWindow = app->activeViewWindow();
    if (!viewWindow)
    {
        RiaLogging::error("No view window is available, nothing to do");
        
        return;
    }

    QString startPath;
    if (!proj->fileName().isEmpty())
    {
        QFileInfo fi(proj->fileName());
        startPath = fi.absolutePath();
    }
    else
    {
        startPath = app->lastUsedDialogDirectory("IMAGE_SNAPSHOT");
    }

    startPath = caf::Utils::constructFullFileName(startPath, RicSnapshotFilenameGenerator::generateSnapshotFileName(viewWindow), ".png");

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Export to File"), startPath);
    if (fileName.isEmpty())
    {
        return;
    }

    // Remember the directory to next time
    app->setLastUsedDialogDirectory("IMAGE_SNAPSHOT", QFileInfo(fileName).absolutePath());

    RicSnapshotViewToFileFeature::saveSnapshotAs(fileName, viewWindow);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot To File");
    actionToSetup->setIcon(QIcon(":/SnapShotSave.png"));
}
