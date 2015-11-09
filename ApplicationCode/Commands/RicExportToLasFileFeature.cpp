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

#include "RimWellLogPlotCurve.h"
#include "RigWellLogFile.h"

#include "RiuMainWindow.h"
#include "RiaApplication.h"

#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileDialog>
#include <QRegExp>

CAF_CMD_SOURCE_INIT(RicExportToLasFileFeature, "RicExportToLasFileFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportToLasFileFeature::isCommandEnabled()
{
    return selectedWellLogPlotCurve() != NULL;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::onActionTriggered(bool isChecked)
{
    RimWellLogPlotCurve* curve = selectedWellLogPlotCurve();
    if (curve)
    {
        QString defaultDir = RiaApplication::instance()->defaultFileDialogDirectory("WELL_LOGS_DIR");
 
        QString defaultFileName = curve->name().trimmed();
        defaultFileName.replace(".", "_");
        defaultFileName.replace(",", "_");
        defaultFileName.replace(" ", "_");
        defaultFileName.replace(QRegExp("_+"), "_");
        defaultFileName.append(".las");

        QString fileName = QFileDialog::getSaveFileName(RiuMainWindow::instance(), tr("Export Curve Data To LAS File"), QDir(defaultDir).absoluteFilePath(defaultFileName), tr("LAS Files (*.las);;All files(*.*)"));
        if (!fileName.isEmpty())
        {
            RigWellLogFile::exportToLasFile(curve, fileName);
        }
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
RimWellLogPlotCurve* RicExportToLasFileFeature::selectedWellLogPlotCurve() const
{
    std::vector<RimWellLogPlotCurve*> selection;
    caf::SelectionManager::instance()->objectsByType(&selection);

    if (selection.size() > 0)
    {
        return selection[0];
    }

    return NULL;
}
