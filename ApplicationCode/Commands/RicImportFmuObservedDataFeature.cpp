/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019- Equinor ASA
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
#include "RicImportFmuObservedDataFeature.h"

#include "RiaApplication.h"
#include "RifReaderFmuRft.h"

#include "RimObservedData.h"
#include "RimObservedDataCollection.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFmuObservedDataFeature::selectObservedDataPathInDialog()
{
    RiaApplication* app       = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("SUMMARY_CASE_DIR");
    QString directory  = QFileDialog::getExistingDirectory(
        nullptr, "Import Fmu Observed Data Recursively from Directory", defaultDir, QFileDialog::ShowDirsOnly);

	QStringList subDirsWithFmuData = RifReaderFmuRft::findSubDirectoriesWithFmuRftData(directory);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportFmuObservedDataFeature::isCommandEnabled()
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
void RicImportFmuObservedDataFeature::onActionTriggered(bool isChecked) {}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportFmuObservedDataFeature::setupActionLook(QAction* actionToSetup) {}
