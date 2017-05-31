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
#include "RiaLogging.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimViewWindow.h"
#include "RiuMainPlotWindow.h"

#include "cafUtils.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>
#include <QMessageBox>


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
    this->disableModelChangeContribution();

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
void RicSnapshotViewToFileFeature::saveSnapshotAs(const QString& fileName, RimViewWindow* viewWindow)
{
    if (viewWindow)
    {
        QImage image = viewWindow->snapshotWindowContent();
        if (!image.isNull())
        {
            if (image.save(fileName))
            {
                qDebug() << "Exported snapshot image to " << fileName;
            }
            else
            {
                qDebug() << "Error when trying to export snapshot image to " << fileName;
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

    startPath += "/image.png";

    QString fileName = QFileDialog::getSaveFileName(NULL, tr("Export to File"), startPath, tr("Image files (*.bmp *.png * *.jpg)"));
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





CAF_CMD_SOURCE_INIT(RicSnapshotAllPlotsToFileFeature, "RicSnapshotAllPlotsToFileFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::saveAllPlots()
{
    RiaApplication* app = RiaApplication::instance();

    RiuMainPlotWindow* mainPlotWindow = app->mainPlotWindow();
    if (!mainPlotWindow) return;

    RimProject* proj = app->project();
    if (!proj) return;

    // Save images in snapshot catalog relative to project directory
    QString snapshotFolderName = app->createAbsolutePathFromProjectRelativePath("snapshots");

    exportSnapshotOfAllPlotsIntoFolder(snapshotFolderName);

    QString text = QString("Exported snapshots to folder : \n%1").arg(snapshotFolderName);
    QMessageBox::information(nullptr, "Export Snapshots To Folder", text);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::exportSnapshotOfAllPlotsIntoFolder(QString snapshotFolderName)
{
    RiaApplication* app = RiaApplication::instance();

    RimProject* proj = app->project();
    if (!proj) return;

    QDir snapshotPath(snapshotFolderName);
    if (!snapshotPath.exists())
    {
        if (!snapshotPath.mkpath(".")) return;
    }

    const QString absSnapshotPath = snapshotPath.absolutePath();

    std::vector<RimViewWindow*> viewWindows;
    proj->mainPlotCollection()->descendantsIncludingThisOfType(viewWindows);

    for (auto viewWindow : viewWindows)
    {
        if (viewWindow->isMdiWindow() && viewWindow->viewWidget())
        {
            QString fileName;
            if ( viewWindow->userDescriptionField())
            {
                fileName = viewWindow->userDescriptionField()->uiCapability()->uiValue().toString();
            }
            else
            {
                fileName = viewWindow->uiCapability()->uiName();
            }

            fileName = caf::Utils::makeValidFileBasename(fileName);

            QString absoluteFileName = caf::Utils::constructFullFileName(absSnapshotPath, fileName, ".png");
            absoluteFileName.replace(" ", "_");

            RicSnapshotViewToFileFeature::saveSnapshotAs(absoluteFileName, viewWindow);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSnapshotAllPlotsToFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::onActionTriggered(bool isChecked)
{
    QWidget* currentActiveWidget = nullptr;
    if (RiaApplication::activeViewWindow())
    {
        currentActiveWidget = RiaApplication::activeViewWindow()->viewWidget();
    }

    RicSnapshotAllPlotsToFileFeature::saveAllPlots();

    if (currentActiveWidget)
    {
        RiuMainPlotWindow* mainPlotWindow = RiaApplication::instance()->mainPlotWindow();
        if (mainPlotWindow)
        {
            mainPlotWindow->setActiveViewer(currentActiveWidget);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Snapshot All Plots To File");
    actionToSetup->setIcon(QIcon(":/SnapShotSaveViews.png"));
}

