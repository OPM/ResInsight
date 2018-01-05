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

#include "RicWellPathsImportSsihubFeature.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellPathImport.h"
#include "RiuMainWindow.h"
#include "RiuWellImportWizard.h"

#include "cafUtils.h"

#include <QAction>
#include <QDir>
#include <QFile>

CAF_CMD_SOURCE_INIT(RicWellPathsImportSsihubFeature, "RicWellPathsImportSsihubFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathsImportSsihubFeature::isCommandEnabled()
{
    RiaApplication* app = RiaApplication::instance();
    if (!app->project())
    {
        return false;
    }

    if (!caf::Utils::fileExists(app->project()->fileName()))
    {
        return false;
    }
    
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathsImportSsihubFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    if (!app->project()) return;

    if (!caf::Utils::fileExists(app->project()->fileName())) return;


    // Update the UTM bounding box from the reservoir
    app->project()->computeUtmAreaOfInterest();

    QString wellPathsFolderPath = RimTools::getCacheRootDirectoryPathFromProject();
    wellPathsFolderPath += "_wellpaths";
    QDir::root().mkpath(wellPathsFolderPath);

    if (!app->project()->wellPathImport()) return;

    // Keep a copy of the import settings, and restore if cancel is pressed in the import wizard
    QString copyOfOriginalObject = app->project()->wellPathImport()->writeObjectToXmlString();

    if (!app->preferences()) return;
    RiuWellImportWizard wellImportwizard(app->preferences()->ssihubAddress, wellPathsFolderPath, app->project()->wellPathImport(), RiuMainWindow::instance());

    // Get password/username from application cache
    {
#ifdef _DEBUG
        // Valid credentials for ssihubfake received in mail from Håkon 
        QString ssihubUsername = "admin";
        QString ssihubPassword = "resinsight";
#else
        QString ssihubUsername = app->cacheDataObject("ssihub_username").toString();
        QString ssihubPassword = app->cacheDataObject("ssihub_password").toString();
#endif
        wellImportwizard.setCredentials(ssihubUsername, ssihubPassword);
    }

    if (QDialog::Accepted == wellImportwizard.exec())
    {
        QStringList wellPaths = wellImportwizard.absoluteFilePathsToWellPaths();
        if (wellPaths.size() > 0)
        {
            app->addWellPathsToModel(wellPaths);
            app->project()->createDisplayModelAndRedrawAllViews();
        }

        app->setCacheDataObject("ssihub_username", wellImportwizard.field("username"));
        app->setCacheDataObject("ssihub_password", wellImportwizard.field("password"));
    }
    else
    {
        app->project()->wellPathImport()->readObjectFromXmlString(copyOfOriginalObject, caf::PdmDefaultObjectFactory::instance());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathsImportSsihubFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Import Well Paths from &SSI-hub");
    actionToSetup->setIcon(QIcon(":/WellCollection.png"));
}
