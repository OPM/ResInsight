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

#include "RicExportWellPathFractureWellCompletionFeature.h"


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
#include "RimWellPathCollection.h"
#include "RimView.h"

CAF_CMD_SOURCE_INIT(RicExportWellPathFractureWellCompletionFeature, "RicExportWellPathFractureWellCompletionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportWellPathFractureWellCompletionFeature::onActionTriggered(bool isChecked)
{

    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return;

    RimWellPathCollection* wellpathColl = nullptr;
    objHandle->firstAncestorOrThisOfType(wellpathColl);

    RimFractureExportSettings exportSettings;

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    if (projectFolder.isEmpty())
    {
        RimView* activeView = RiaApplication::instance()->activeReservoirView();
        if (!activeView) return;
        RimEclipseView * activeRiv = dynamic_cast<RimEclipseView*>(activeView);
        if (!activeRiv) return;
        projectFolder = activeRiv->eclipseCase()->locationOnDisc();
    }

    QString outputFileName = projectFolder + "/Fractures";
    exportSettings.fileName = outputFileName;

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Fracture Well Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        bool isOk = RifEclipseExportTools::writeWellPathFracturesToTextFile(exportSettings.fileName, wellpathColl);

        if (!isOk)
        {
            QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + exportSettings.fileName);
        }
    }


}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportWellPathFractureWellCompletionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureTemplate16x16.png"));
    actionToSetup->setText("Export Fracture Well Completion Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportWellPathFractureWellCompletionFeature::isCommandEnabled()
{
    return true;
}
