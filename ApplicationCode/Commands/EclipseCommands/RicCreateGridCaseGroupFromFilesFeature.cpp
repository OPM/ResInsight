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

#include "RicCreateGridCaseGroupFromFilesFeature.h"

#include "RiaApplication.h"
#include "RiaImportEclipseCaseTools.h"

#include "RicFileHierarchyDialog.h"

#include "RimEclipseCaseCollection.h"
#include "RiuMultiCaseImportDialog.h"

#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicCreateGridCaseGroupFromFilesFeature, "RicCreateGridCaseGroupFromFilesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCreateGridCaseGroupFromFilesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseGroupFromFilesFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");

    RicFileHierarchyDialogResult result = RicFileHierarchyDialog::getOpenFileNames(nullptr, "Create Grid Case Group from Files", defaultDir, m_pathFilter, m_fileNameFilter, QStringList(".EGRID"));

    // Remember filters
    m_pathFilter = result.pathFilter;
    m_fileNameFilter = result.fileNameFilter;

    if (result.ok)
    {
        // Remember the path to next time
        app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(result.rootDir).absoluteFilePath());

        RiaImportEclipseCaseTools::addEclipseCases(result.files);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseGroupFromFilesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CreateGridCaseGroup16x16.png"));
    actionToSetup->setText("&Create Grid Case Group from Files Recursively");
}
