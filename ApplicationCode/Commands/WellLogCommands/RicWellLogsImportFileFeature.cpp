/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicWellLogsImportFileFeature.h"

#include "RiaApplication.h"
#include "RimProject.h"
#include "RiuMainWindow.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicWellLogsImportFileFeature, "RicWellLogsImportFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellLogsImportFileFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogsImportFileFeature::onActionTriggered(bool isChecked)
{
    // Open dialog box to select well path files
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("WELL_LOGS_DIR");
    QStringList wellLogFilePaths = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Import Well Logs", defaultDir, "Well Logs (*.las);;All Files (*.*)");

    if (wellLogFilePaths.size() < 1) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("WELL_LOGS_DIR", QFileInfo(wellLogFilePaths.last()).absolutePath());

    app->addWellLogsToModel(wellLogFilePaths);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogsImportFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Import Well &Logs from File");
    actionToSetup->setIcon(QIcon(":/LasFile16x16.png"));
}
