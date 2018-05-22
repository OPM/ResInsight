/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportGeoMechCaseFeature.h"

#include "RiaApplication.h"
  
#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportGeoMechCaseFeature, "RicImportGeoMechCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportGeoMechCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportGeoMechCaseFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectory("GEOMECH_MODEL");
    QStringList fileNames = QFileDialog::getOpenFileNames(nullptr, "Import Geo-Mechanical Model", defaultDir, "Abaqus results (*.odb)");
    if (fileNames.size()) defaultDir = QFileInfo(fileNames.last()).absolutePath();
    app->setLastUsedDialogDirectory("GEOMECH_MODEL", defaultDir);

    for (int i = 0; i < fileNames.size(); i++)
    {
        QString fileName = fileNames[i];

        if (!fileNames.isEmpty())
        {
            if (app->openOdbCaseFromFile(fileName))
            {
                app->addToRecentFiles(fileName);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportGeoMechCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/GeoMechCase48x48.png"));
    actionToSetup->setText("Import &Geo Mechanical Model");
}
