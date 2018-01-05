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

#include "RicImportInputEclipseCaseFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPorosityModel.h"

#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportInputEclipseCaseFeature, "RicImportInputEclipseCaseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportInputEclipseCaseFeature::openInputEclipseCaseFromFileNames(const QStringList& fileNames)
{
    RimEclipseInputCase* rimInputReservoir = new RimEclipseInputCase();

    RiaApplication* app = RiaApplication::instance();
    RimProject* project = app->project();

    project->assignCaseIdToCase(rimInputReservoir);

    bool gridImportSuccess = rimInputReservoir->openDataFileSet(fileNames);
    if (!gridImportSuccess)
    {
        RiaLogging::error("Failed to import grid");
        return false;
    }

    RimEclipseCaseCollection* analysisModels = project->activeOilField() ? project->activeOilField()->analysisModels() : nullptr;
    if (analysisModels == nullptr) return false;

    analysisModels->cases.push_back(rimInputReservoir);

    RimEclipseView* riv = rimInputReservoir->createAndAddReservoirView();

    riv->cellResult()->setResultType(RiaDefines::INPUT_PROPERTY);
    riv->hasUserRequestedAnimation = true;

    riv->loadDataAndUpdate();

    if (!riv->cellResult()->hasResult())
    {
        riv->cellResult()->setResultVariable(RiaDefines::undefinedResultName());
    }

    analysisModels->updateConnectedEditors();

    RiuMainWindow::instance()->selectAsCurrentItem(riv->cellResult());

    if (fileNames.size() == 1)
    {
        app->addToRecentFiles(fileNames[0]);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportInputEclipseCaseFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportInputEclipseCaseFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("INPUT_FILES");
    QStringList fileNames = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Import Eclipse Input Files", defaultDir, "Eclipse Input Files and Input Properties Eclipse Input Files (*.GRDECL);;All Files (*.*)");

    if (fileNames.isEmpty()) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("INPUT_FILES", QFileInfo(fileNames.last()).absolutePath());

    RicImportInputEclipseCaseFeature::openInputEclipseCaseFromFileNames(fileNames);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportInputEclipseCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/EclipseInput48x48.png"));
    actionToSetup->setText("Import Input Eclipse Case");
}
