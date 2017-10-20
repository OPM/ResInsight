/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicImportFormationNamesFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimFormationNames.h"
#include "RimFormationNamesCollection.h"
#include "RimGeoMechCase.h"
#include "RimOilField.h"
#include "RimProject.h"

#include "RigEclipseCaseData.h"
#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RiuMainWindow.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportFormationNamesFeature, "RicImportFormationNamesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicImportFormationNamesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString defaultDir = app->lastUsedDialogDirectory("BINARY_GRID");
    QStringList fileNames = QFileDialog::getOpenFileNames(RiuMainWindow::instance(), "Import Formation Names", defaultDir, "Formation Names description File (*.lyr);;All Files (*.*)");

    if (fileNames.isEmpty()) return;

    // Remember the path to next time
    app->setLastUsedDialogDirectory("BINARY_GRID", QFileInfo(fileNames.last()).absolutePath());

    // Find or create the FomationNamesCollection

    RimProject* proj = RiaApplication::instance()->project();
    RimFormationNamesCollection* fomNameColl = proj->activeOilField()->formationNamesCollection();
    if (!fomNameColl)
    {
        fomNameColl = new RimFormationNamesCollection;
        proj->activeOilField()->formationNamesCollection = fomNameColl;
    }

    // For each file, find existing Formation names item, or create new

    RimFormationNames* formationName = fomNameColl->importFiles(fileNames);

    std::vector<RimCase*> cases;
    proj->allCases(cases);
    
    if (cases.size() == 1)
    {
        std::vector<RimEclipseCase*> eclCases = proj->eclipseCases();
        if (eclCases.size() == 1)
        {
            eclCases[0]->activeFormationNames = formationName;
            eclCases[0]->eclipseCaseData()->setActiveFormationNames(formationName->formationNamesData());
        }

        std::vector<RimGeoMechCase*> geoMechCases = proj->geoMechCases();

        if (geoMechCases.size() == 1)
        {
            geoMechCases[0]->activeFormationNames = formationName;
            geoMechCases[0]->geoMechData()->femPartResults()->setActiveFormationNames(formationName->formationNamesData());
        }
    }

    proj->updateConnectedEditors();

    if (formationName)
    {
        RiuMainWindow::instance()->selectAsCurrentItem(formationName);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportFormationNamesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FormationCollection16x16.png"));
    actionToSetup->setText("Import Formation Names");
}
