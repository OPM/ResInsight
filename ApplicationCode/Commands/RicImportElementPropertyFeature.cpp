/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicImportElementPropertyFeature.h"

#include "RiaApplication.h"

#include "RigFemPartResultsCollection.h"
#include "RigGeoMechCaseData.h"

#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportElementPropertyFeature, "RicImportElementPropertyFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicImportElementPropertyFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportElementPropertyFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();

    QString     defaultDir = app->lastUsedDialogDirectory("ELM_PROPS");
    QStringList fileNames =
        QFileDialog::getOpenFileNames(NULL, "Import Element Property Table", defaultDir, "Property Table (*.inp)");

    if (fileNames.size())
    {
        defaultDir = QFileInfo(fileNames.last()).absolutePath();
    }

    std::vector<caf::FilePath> filePaths;
    for (QString filename : fileNames)
    {
        filePaths.push_back(caf::FilePath(filename));
    }

    app->setLastUsedDialogDirectory("ELM_PROPS", defaultDir);

    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    if (!activeView) return;
    
    RimGeoMechView* activeGmv = dynamic_cast<RimGeoMechView*>(activeView);
    if (!activeGmv) return;

    if (activeGmv->geoMechCase())
    {
        activeGmv->geoMechCase()->addElementPropertyFiles(filePaths);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicImportElementPropertyFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/GeoMechCase48x48.png"));
    actionToSetup->setText("Import &Element Property Table");
}
