/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017  Statoil ASA
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

#include "RicImportEclipseCaseTimeStepFilterFeature.h"

#include "RiaApplication.h"
#include "RiaImportEclipseCaseTools.h"

#include "RiuMainWindow.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicImportEclipseCaseTimeStepFilterFeature, "RicImportEclipseCaseTimeStepFilterFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEclipseCaseTimeStepFilterFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");
    QString fileName = QFileDialog::getOpenFileName(RiuMainWindow::instance(), "Import Eclipse File", defaultDir, "Eclipse Grid Files (*.GRID *.EGRID)");
    if (!fileName.isEmpty())
    {
        defaultDir = QFileInfo(fileName).absolutePath();
        app->setLastUsedDialogDirectory("BINARY_GRID", defaultDir);

        if (RiaImportEclipseCaseTools::openEclipseCaseShowTimeStepFilter(fileName))
        {
            app->addToRecentFiles(fileName);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEclipseCaseTimeStepFilterFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Case48x48.png"));
    actionToSetup->setText("Import Eclipse Case (Time Step Filtered)");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEclipseCaseTimeStepFilterFeature::isCommandEnabled()
{
    return true;
}
