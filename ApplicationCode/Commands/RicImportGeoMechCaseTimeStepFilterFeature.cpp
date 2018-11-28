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

#include "RicImportGeoMechCaseTimeStepFilterFeature.h"

#include "RiaApplication.h"
#include "RiaImportEclipseCaseTools.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>
#include <QFileDialog>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicImportGeoMechCaseTimeStepFilterFeature, "RicImportGeoMechCaseTimeStepFilterFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportGeoMechCaseTimeStepFilterFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectory("GEOMECH_MODEL");
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr, "Import Geo-Mechanical Model", defaultDir, "Abaqus results (*.odb)");
    if (fileNames.size()) defaultDir = QFileInfo(fileNames.last()).absolutePath();
    for (QString fileName : fileNames)
    {
        if (!fileName.isEmpty())
        {
            defaultDir = QFileInfo(fileName).absolutePath();
            app->setLastUsedDialogDirectory("GEOMECH_MODEL", defaultDir);
            if (app->openOdbCaseFromFile(fileName, true))
            {
                app->addToRecentFiles(fileName);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportGeoMechCaseTimeStepFilterFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/GeoMechCase48x48.png"));
    actionToSetup->setText("Import Geo Mechanical Model (Time Step Filtered)");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportGeoMechCaseTimeStepFilterFeature::isCommandEnabled()
{
    return true;
}
