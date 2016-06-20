/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016 Statoil ASA
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

#include "RicImportInputEclipseCaseOpmFeature.h"

#include "RiaApplication.h"

#include "RimDefines.h"
#include "RimEclipseCaseCollection.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCaseOpm.h"
#include "RimEclipseView.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportInputEclipseCaseOpmFeature, "RicImportInputEclipseCaseOpmFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportInputEclipseCaseOpmFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportInputEclipseCaseOpmFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->defaultFileDialogDirectory("INPUT_FILES");
    QString fileName = QFileDialog::getOpenFileName(RiuMainWindow::instance(), "Import Eclipse DATA file", defaultDir, "Eclipse Input Files and Input Properties (*.DATA *)");

    if (fileName.isEmpty()) return;

    // Remember the path to next time
    app->setDefaultFileDialogDirectory("INPUT_FILES", QFileInfo(fileName).absolutePath());

    RimProject* proj = app->project();
    RimEclipseCaseCollection* analysisModels = proj->activeOilField() ? proj->activeOilField()->analysisModels() : NULL;
    if (analysisModels)
    {
        // This code originates from RiaApplication::openInputEclipseCaseFromFileNames

        RimEclipseInputCaseOpm* rimInputReservoir = new RimEclipseInputCaseOpm();
        proj->assignCaseIdToCase(rimInputReservoir);

        rimInputReservoir->importNewEclipseGridAndProperties(fileName);

        analysisModels->cases.push_back(rimInputReservoir);

        RimEclipseView* riv = rimInputReservoir->createAndAddReservoirView();

        riv->cellResult()->setResultType(RimDefines::INPUT_PROPERTY);
        riv->hasUserRequestedAnimation = true;

        riv->loadDataAndUpdate();

        if (!riv->cellResult()->hasResult())
        {
            riv->cellResult()->setResultVariable(RimDefines::undefinedResultName());
        }

        analysisModels->updateConnectedEditors();

        RiuMainWindow::instance()->selectAsCurrentItem(riv->cellResult());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportInputEclipseCaseOpmFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/EclipseInput48x48.png"));
    actionToSetup->setText("Import Input Eclipse Case (opm-parser) - BETA");
}
