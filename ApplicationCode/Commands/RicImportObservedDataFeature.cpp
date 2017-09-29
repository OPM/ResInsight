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

#include "RicImportObservedDataFeature.h"

#include "RiaApplication.h"

#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"
#include "RimSummaryObservedDataFile.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>


CAF_CMD_SOURCE_INIT(RicImportObservedDataFeature, "RicImportObservedDataFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicImportObservedDataFeature::RicImportObservedDataFeature()
{
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportObservedDataFeature::selectObservedDataFileInDialog()
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");
    QStringList fileNames = QFileDialog::getOpenFileNames(NULL, "Import Observed Time History Data", defaultDir, "All Files (*.*)");

    if (fileNames.isEmpty()) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(fileNames.last()).absolutePath());

    RimProject* proj = app->project();
    RimObservedDataCollection* observedDataCollection = proj->activeOilField() ? proj->activeOilField()->observedDataCollection() : nullptr;
    if (!observedDataCollection) return;

    for (auto fileName : fileNames)
    {
        RicImportObservedDataFeature::createAndAddObservedDataFromFile(fileName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportObservedDataFeature::isCommandEnabled()
{
    std::vector<RimObservedDataCollection*> selectionObservedDataCollection;
    caf::SelectionManager::instance()->objectsByType(&selectionObservedDataCollection);

    std::vector<RimObservedData*> selectionObservedData;
    caf::SelectionManager::instance()->objectsByType(&selectionObservedData);

    return (selectionObservedDataCollection.size() > 0 || selectionObservedData.size() > 0);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportObservedDataFeature::onActionTriggered(bool isChecked)
{
    RicImportObservedDataFeature::selectObservedDataFileInDialog();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportObservedDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/Default.png"));
    actionToSetup->setText("Import Observed Time History Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportObservedDataFeature::createAndAddObservedDataFromFile(const QString& fileName)
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    RimObservedDataCollection* observedDataCollection = proj->activeOilField() ? proj->activeOilField()->observedDataCollection() : nullptr;
    if (!observedDataCollection) return false;

    RimObservedData* newObservedData = observedDataCollection->createAndAddObservedDataFromFileName(fileName);

    return true;
}

