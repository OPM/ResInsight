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

#include "RicSnapshotAllPlotsToFileFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimViewWindow.h"

#include "RicSnapshotViewToFileFeature.h"
#include "RicSnapshotFilenameGenerator.h"

#include "RiuPlotMainWindowTools.h"

#include "cafUtils.h"

#include <QAction>
#include <QClipboard>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMdiSubWindow>


CAF_CMD_SOURCE_INIT(RicSnapshotAllPlotsToFileFeature, "RicSnapshotAllPlotsToFileFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSnapshotAllPlotsToFileFeature::saveAllPlots()
{
    RiaApplication* app = RiaApplication::instance();

    RiuPlotMainWindow* mainPlotWindow = app->mainPlotWindow();
    if (!mainPlotWindow) return;

    RimProject* proj = app->project();
    if (!proj) return;

    // Save images in snapshot catalog relative to project directory
    QString snapshotFolderName = app->createAbsolutePathFromProjectRelativePath("snapshots");

    exportSnapshotOfAllPlotsIntoFolder(snapshotFolderName);

    QString text = QString("Exported snapshots to folder : \n%1").arg(snapshotFolderName);
    RiaLogging::info(text);
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
            QString fileName = RicSnapshotFilenameGenerator::generateSnapshotFileName(viewWindow);
            fileName.replace(" ", "_");

            QString absoluteFileName = caf::Utils::constructFullFileName(absSnapshotPath, fileName, ".png");

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
        RiuPlotMainWindowTools::setActiveViewer(currentActiveWidget);
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

