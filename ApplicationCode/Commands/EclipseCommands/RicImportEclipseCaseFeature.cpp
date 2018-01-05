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

#include "RicImportEclipseCaseFeature.h"

#include "RiaImportEclipseCaseTools.h"

#include "RiaApplication.h"

#include "RimEclipseCaseCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportEclipseCaseFeature, "RicImportEclipseCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEclipseCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEclipseCaseFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");
    QStringList fileNames = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Import Eclipse File", defaultDir, "Eclipse Grid Files (*.GRID *.EGRID)");
    if (fileNames.size()) defaultDir = QFileInfo(fileNames.last()).absolutePath();
    app->setLastUsedDialogDirectory("BINARY_GRID", defaultDir);

    int i;
    for (i = 0; i < fileNames.size(); i++)
    {
        QString fileName = fileNames[i];

        if (!fileNames.isEmpty())
        {
            if (RiaImportEclipseCaseTools::openEclipseCaseFromFile(fileName))
            {
                app->addToRecentFiles(fileName);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEclipseCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Case48x48.png"));
    actionToSetup->setText("Import Eclipse Case");
}
