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

#include "RicCreateGridCaseGroupFeature.h"

#include "RimEclipseCaseCollection.h"
#include "RiaApplication.h"
#include "RiuMultiCaseImportDialog.h"

#include "cafSelectionManager.h"
  
#include <QAction>

CAF_CMD_SOURCE_INIT(RicCreateGridCaseGroupFeature, "RicCreateGridCaseGroupFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicCreateGridCaseGroupFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseGroupFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    RiuMultiCaseImportDialog dialog;
    int action = dialog.exec();
    if (action == QDialog::Accepted)
    {
        QStringList gridFileNames = dialog.eclipseCaseFileNames();
        app->addEclipseCases(gridFileNames);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicCreateGridCaseGroupFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CreateGridCaseGroup16x16.png"));
    actionToSetup->setText("&Create Grid Case Group from Files");
}
