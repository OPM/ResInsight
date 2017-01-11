/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicExportSimWellFractureWellCompletionFeature.h"


#include "RiaApplication.h"

#include "RifEclipseExportTools.h"
#include "RifEclipseExportTools.h"

#include "RimBinaryExportSettings.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimFractureExportSettings.h"

#include "RiuMainWindow.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QMessageBox>
#include <QString>

CAF_CMD_SOURCE_INIT(RicExportSimWellFractureWellCompletionFeature, "RicExportSimWellFractureWellCompletionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSimWellFractureWellCompletionFeature::onActionTriggered(bool isChecked)
{

    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimEclipseWellCollection* eclipseWellColl = nullptr;
    objHandle->firstAncestorOrThisOfType(eclipseWellColl);

    RimEclipseView* eclipseWiew = nullptr;
    objHandle->firstAncestorOrThisOfType(eclipseWiew);

    RimFractureExportSettings exportSettings;

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();
    if (projectFolder.isEmpty())
    {
        projectFolder = eclipseWiew->eclipseCase()->locationOnDisc();
    }

    QString outputFileName = projectFolder + "/Fractures";
    exportSettings.fileName = outputFileName;

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Fracture Well Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        bool isOk = RifEclipseExportTools::writeSimWellFracturesToTextFile(exportSettings.fileName, eclipseWellColl);

        if (!isOk)
        {
            QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + exportSettings.fileName);
        }
    }


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSimWellFractureWellCompletionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));
    actionToSetup->setText("Export Fracture Well Completion Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportSimWellFractureWellCompletionFeature::isCommandEnabled()
{
    return true;
}
