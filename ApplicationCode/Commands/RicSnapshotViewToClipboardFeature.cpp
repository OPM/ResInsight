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

#include "RicSnapshotViewToClipboardFeature.h"

#include "RiaApplication.h"

#include "RimProject.h"
#include "RimViewWindow.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>


CAF_CMD_SOURCE_INIT(RicSnapshotViewToClipboardFeature, "RicSnapshotViewToClipboardFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSnapshotViewToClipboardFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToClipboardFeature::onActionTriggered(bool isChecked)
{
    RimViewWindow* viewWindow = RiaApplication::activeViewWindow();

    if (viewWindow)
    {
        QClipboard* clipboard = QApplication::clipboard();
        if (clipboard)
        {
            QImage image = viewWindow->snapshotWindowContent();
            if (!image.isNull())
            {
                clipboard->setImage(image);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToClipboardFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot To Clipboard");
    actionToSetup->setIcon(QIcon(":/SnapShot.png"));
}




CAF_CMD_SOURCE_INIT(RicSnapshotViewToFileFeature, "RicSnapshotViewToFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::saveSnapshotAs(const QString& fileName)
{
    RimViewWindow* viewWindow = RiaApplication::activeViewWindow();

    if (viewWindow)
    {
        QImage image = viewWindow->snapshotWindowContent();
        if (!image.isNull())
        {
            if (image.save(fileName))
            {
                qDebug() << "Saved snapshot image to " << fileName;
            }
            else
            {
                qDebug() << "Error when trying to save snapshot image to " << fileName;
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

    startPath += "/image.png";

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Save File"), startPath, tr("Image files (*.bmp *.png * *.jpg)"));
    if (fileName.isEmpty())
    {
        return;
    }

    // Remember the directory to next time
    app->setLastUsedDialogDirectory("IMAGE_SNAPSHOT", QFileInfo(fileName).absolutePath());

    RicSnapshotViewToFileFeature::saveSnapshotAs(fileName);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotViewToFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot To File");
    actionToSetup->setIcon(QIcon(":/SnapShotSave.png"));
}

