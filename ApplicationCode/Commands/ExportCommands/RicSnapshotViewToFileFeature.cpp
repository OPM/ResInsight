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
#include "RiuPlotMainWindow.h"

#include "RicSnapshotFilenameGenerator.h"

#include "cafUtils.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>


CAF_CMD_SOURCE_INIT(RicSnapshotViewToFileFeature, "RicSnapshotViewToFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveSnapshotAs(const QString& fileName, RimViewWindow* viewWindow)
{
    if (viewWindow)
    {
        QImage image = viewWindow->snapshotWindowContent();
        saveSnapshotAs(fileName, image);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveSnapshotAs(const QString& fileName, const QImage& image)
{
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveToFile(const QImage& image, const QString& defaultFileBaseName)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

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

    QString defaultAbsFileName = caf::Utils::constructFullFileName(startPath, defaultFileBaseName, ".png");
    QString fileName = QFileDialog::getSaveFileName(nullptr, tr("Export to File"), defaultAbsFileName);
    if (fileName.isEmpty())
    {
        return;
    }

    // Remember the directory to next time
    app->setLastUsedDialogDirectory("IMAGE_SNAPSHOT", QFileInfo(fileName).absolutePath());

    RicSnapshotViewToFileFeature::saveSnapshotAs(fileName, image);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QIcon RicSnapshotViewToFileFeature::icon()
{
    return QIcon(":/SnapShotSave.png");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicSnapshotViewToFileFeature::text()
{
    return "Snapshot To File";
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
    // Get active view window before displaying the file selection dialog
    // If this is done after the file save dialog is displayed (and closed)
    // app->activeViewWindow() returns NULL on Linux

    RimViewWindow* viewWindow = RiaApplication::activeViewWindow();
    if (!viewWindow)
    {
        RiaLogging::error("No view window is available, nothing to do");
        
        return;
    }

    QImage image = viewWindow->snapshotWindowContent();
    saveToFile(image, RicSnapshotFilenameGenerator::generateSnapshotFileName(viewWindow));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText(text());
    actionToSetup->setIcon(icon());
}
