/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicWellPathImportCompletionsFileFeature.h"

#include "RiaApplication.h"

#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCompletions.h"
#include "RimWellPathCollection.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicWellPathImportCompletionsFileFeature, "RicWellPathImportCompletionsFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathImportCompletionsFileFeature::isCommandEnabled()
{
    if (RicWellPathImportCompletionsFileFeature::selectedWellPathCollection() != nullptr)
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathImportCompletionsFileFeature::onActionTriggered(bool isChecked)
{
    RimFishboneWellPathCollection* fishbonesWellPathCollection = RicWellPathImportCompletionsFileFeature::selectedWellPathCollection();
    CVF_ASSERT(fishbonesWellPathCollection);

    // Open dialog box to select well path files
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("WELLPATH_DIR");
    QStringList wellPathFilePaths = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Import Well Path Completions", defaultDir, "Well Path Completions (*.json *.asc *.asci *.ascii *.dev);;All Files (*.*)");

    if (wellPathFilePaths.size() < 1) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("WELLPATH_DIR", QFileInfo(wellPathFilePaths.last()).absolutePath());

    fishbonesWellPathCollection->importCompletionsFromFile(wellPathFilePaths);

    RimWellPathCollection* wellPathCollection;
    fishbonesWellPathCollection->firstAncestorOrThisOfType(wellPathCollection);
    if (wellPathCollection)
    {
        wellPathCollection->updateConnectedEditors();
    }

    if (app->project())
    {
        app->project()->createDisplayModelAndRedrawAllViews();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathImportCompletionsFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Import Completions from File");
    actionToSetup->setIcon(QIcon(":/FishBoneGroupFromFile16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishboneWellPathCollection* RicWellPathImportCompletionsFileFeature::selectedWellPathCollection()
{
    RimFishbonesCollection* objToFind = nullptr;
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    if (objToFind == nullptr)
    {
        std::vector<RimWellPath*> wellPaths;
        caf::SelectionManager::instance()->objectsByType(&wellPaths);
        if (!wellPaths.empty())
        {
            return wellPaths[0]->fishbonesCollection()->wellPathCollection();
        }
    }
    else
    {
        return objToFind->wellPathCollection();
    }

    return nullptr;
}
