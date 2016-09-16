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

#include "RicExportToLasFileFeature.h"

#include "RiaApplication.h"
#include "RigLasFileExporter.h"
#include "RimWellLogCurve.h"

#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicExportToLasFileFeature, "RicExportToLasFileFeature");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportToLasFileFeature::isCommandEnabled()
{
    return selectedWellLogPlotCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellLogCurve*> curves = selectedWellLogPlotCurves();
    if (curves.size() == 0) return;

    QString defaultDir = RiaApplication::instance()->defaultFileDialogDirectory("WELL_LOGS_DIR");
    QString exportFolder = QFileDialog::getExistingDirectory(NULL, "Select destination folder for LAS export");
    if (!exportFolder.isEmpty())
    {
        RigLasFileExporter lasExporter(curves);
        lasExporter.writeToFolder(exportFolder);

        // Remember the path to next time
        RiaApplication::instance()->setDefaultFileDialogDirectory("WELL_LOGS_DIR", exportFolder);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export To LAS File...");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RicExportToLasFileFeature::selectedWellLogPlotCurves() const
{
    std::vector<RimWellLogCurve*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    return selection;
}

