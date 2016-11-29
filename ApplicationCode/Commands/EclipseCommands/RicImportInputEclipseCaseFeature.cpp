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

#include "RimEclipseCaseCollection.h"
#include "RiaApplication.h"
#include "RiuMainWindow.h"

#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicImportInputEclipseCaseFeature, "RicImportInputEclipseCaseFeature");

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

    app->openInputEclipseCaseFromFileNames(fileNames);

}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicImportInputEclipseCaseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/EclipseInput48x48.png"));
    actionToSetup->setText("Import Input Eclipse Case");
}
