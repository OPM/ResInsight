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

#include "RifFractureExportTools.h"
#include "RifFractureExportTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFracture.h"
#include "RimFractureExportSettings.h"
#include "RimView.h"
#include "RimWellPathCollection.h"
#include "RiuMainWindow.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QMessageBox>
#include <QString>
#include <QFileInfo>

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
    std::vector<RimFracture*> fractures;
    wellpathColl->descendantsIncludingThisOfType(fractures);

    RimFractureExportSettings exportSettings;

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();
    
    RimView* view = app->activeReservoirView();
    objHandle = dynamic_cast<caf::PdmObjectHandle*>(view);
    if (!objHandle) return;
    RimEclipseCase* caseToApply;
    objHandle->firstAncestorOrThisOfType(caseToApply);
    exportSettings.caseToApply = caseToApply;

    if (projectFolder.isEmpty())
    {
        RimView* activeView = RiaApplication::instance()->activeReservoirView();
        if (!activeView) return;
        RimEclipseView * activeRiv = dynamic_cast<RimEclipseView*>(activeView);
        if (!activeRiv) return;
        projectFolder = activeRiv->eclipseCase()->locationOnDisc();
    }
    
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("FRACTURE_EXPORT_DIR", projectFolder);

    QString outputFileName = defaultDir + "/Fractures";
    exportSettings.fileName = outputFileName;

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Fracture Well Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("FRACTURE_EXPORT_DIR", QFileInfo(exportSettings.fileName).absolutePath());

        bool isOk = RifFractureExportTools::writeFracturesToTextFile(exportSettings.fileName, fractures, exportSettings.caseToApply);

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
    actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("Export Fracture Well Completion Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportWellPathFractureWellCompletionFeature::isCommandEnabled()
{
    return true;
}
