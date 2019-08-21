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
#include "RicImportObservedFmuDataFeature.h"

#include "RiaApplication.h"
#include "RifReaderFmuRft.h"

#include "RimObservedFmuRftData.h"
#include "RimObservedSummaryData.h"
#include "RimObservedDataCollection.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuPlotMainWindowTools.h"

#include "cafPdmObject.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>


CAF_CMD_SOURCE_INIT(RicImportObservedFmuDataFeature, "RicImportObservedFmuDataFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataFeature::selectObservedDataPathInDialog()
{
    RiaApplication* app       = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("SUMMARY_CASE_DIR");
    QString directory  = QFileDialog::getExistingDirectory(
        nullptr, "Import Observed FMU Data Recursively from Directory", defaultDir, QFileDialog::ShowDirsOnly);

	QStringList subDirsWithFmuData = RifReaderFmuRft::findSubDirectoriesWithFmuRftData(directory);
    if (subDirsWithFmuData.empty()) return;

	RimProject*                proj = app->project();
    RimObservedDataCollection* observedDataCollection =
        proj->activeOilField() ? proj->activeOilField()->observedDataCollection() : nullptr;
    if (!observedDataCollection) return;

	const RimObservedFmuRftData* importedData = nullptr;
	for (const QString& subDir : subDirsWithFmuData)
	{
        importedData = observedDataCollection->createAndAddFmuRftDataFromPath(subDir);		
	}
	if (importedData != nullptr)
	{
        RiuPlotMainWindowTools::showPlotMainWindow();
        RiuPlotMainWindowTools::selectAsCurrentItem(importedData);
	}
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportObservedFmuDataFeature::isCommandEnabled()
{
    std::vector<RimObservedDataCollection*> selectionObservedDataCollection;
    caf::SelectionManager::instance()->objectsByType(&selectionObservedDataCollection);

    std::vector<RimObservedSummaryData*> selectionObservedData;
    caf::SelectionManager::instance()->objectsByType(&selectionObservedData);

    return (selectionObservedDataCollection.size() > 0 || selectionObservedData.size() > 0);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataFeature::onActionTriggered(bool isChecked)
{
    selectObservedDataPathInDialog();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportObservedFmuDataFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/ObservedDataFile16x16.png"));
    actionToSetup->setText("Import Observed FMU Data");
}
