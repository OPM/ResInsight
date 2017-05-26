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

#include "RicExportSelectedWellPathFractureWellCompletionFeature.h"


#include "RiaApplication.h"

#include "RifFractureExportTools.h"
#include "RifFractureExportTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimEclipseWell.h"
#include "RimEclipseWellCollection.h"
#include "RimFracture.h"
#include "RimFractureExportSettings.h"
#include "RimWellPath.h"

#include "RiuMainWindow.h"

#include "cafPdmObjectHandle.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include <QAction>
#include <QMessageBox>
#include <QString>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicExportSelectedWellPathFractureWellCompletionFeature, "RicExportSelectedWellPathFractureWellCompletionFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathFractureWellCompletionFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);
    if (!selection.size()) return;

    RimFractureExportSettings exportSettings;

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    RimView* view = app->activeReservoirView();

    RimEclipseCase* caseToApply;
    view->firstAncestorOrThisOfType(caseToApply);
    if (!caseToApply) return;

    exportSettings.caseToApply = caseToApply;
    if (projectFolder.isEmpty())
    {
        projectFolder = caseToApply->locationOnDisc();
    }

    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("FRACTURE_EXPORT_DIR", projectFolder);

    QString outputFileName = defaultDir + "/Fractures";
    exportSettings.fileName = outputFileName;

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Fracture Well Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("FRACTURE_EXPORT_DIR", QFileInfo(exportSettings.fileName).absolutePath());

        RifFractureExportTools::exportWellPathFracturesToEclipseDataInputFile(exportSettings.fileName, selection[0], exportSettings.caseToApply);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathFractureWellCompletionFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/FractureSymbol16x16.png"));
    actionToSetup->setText("Export Fracture Well Completion Data for Selected wells");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportSelectedWellPathFractureWellCompletionFeature::isCommandEnabled()
{
    return true;
}
