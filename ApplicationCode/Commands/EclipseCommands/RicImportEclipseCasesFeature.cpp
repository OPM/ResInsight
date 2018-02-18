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

#include "RicImportEclipseCasesFeature.h"

#include "RiaImportEclipseCaseTools.h"

#include "RiaApplication.h"

#include "RicFileHierarchyDialog.h"

#include "RimEclipseCaseCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportEclipseCasesFeature, "RicImportEclipseCasesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportEclipseCasesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportEclipseCasesFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");

    RicFileHierarchyDialogResult result = RicFileHierarchyDialog::getOpenFileNames(nullptr, 
                                                                                   "Import Eclipse Cases", 
                                                                                   defaultDir, 
                                                                                   m_pathFilter, 
                                                                                   m_fileNameFilter, 
                                                                                   QStringList() << ".EGRID" << ".GRID");

    // Remember filters
    m_pathFilter = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if (!result.ok) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("BINARY_GRID", QFileInfo(result.rootDir).absoluteFilePath());

    int i;
    for (i = 0; i < result.files.size(); i++)
    {
        QString fileName = result.files[i];

        if (!fileName.isEmpty())
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
void RicImportEclipseCasesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Case48x48.png"));
    actionToSetup->setText("Import Eclipse Cases Recursively");
}
