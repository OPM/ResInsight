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

#include "RicExportToLasFileResampleUi.h"
#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "RiaApplication.h"

#include "RigLasFileExporter.h"
#include "RigWellLogCurveData.h"

#include "RimWellLogCurve.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
  
#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicExportToLasFileFeature, "RicExportToLasFileFeature");



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportToLasFileFeature::isCommandEnabled()
{
    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return false;

    return RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::onActionTriggered(bool isChecked)
{
    this->disableModelChangeContribution();

    if (RicWellLogPlotCurveFeatureImpl::parentWellAllocationPlot()) return;

    std::vector<RimWellLogCurve*> curves = RicWellLogPlotCurveFeatureImpl::selectedWellLogCurves();
    if (curves.size() == 0) return;

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("WELL_LOGS_DIR", projectFolder);

    RigLasFileExporter lasExporter(curves);
    RicExportToLasFileResampleUi featureUi;
    featureUi.exportFolder = defaultDir;

    {
        std::vector<QString> wellNames;
        std::vector<double> rkbDiffs;
        lasExporter.wellPathsAndRkbDiff(&wellNames, &rkbDiffs);
        featureUi.setRkbDiffs(wellNames, rkbDiffs);
    }
    
    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, "Export Curve Data to LAS file(s)", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(400, 200));
    
    if (propertyDialog.exec() == QDialog::Accepted &&
        !featureUi.exportFolder().isEmpty())
    {
        if (featureUi.activateResample)
        {
            lasExporter.setResamplingInterval(featureUi.resampleInterval());
        }

        if (featureUi.exportTvdrkb)
        {
            std::vector<QString> wellNames;
            std::vector<double> rkbDiffs;
            lasExporter.wellPathsAndRkbDiff(&wellNames, &rkbDiffs);

            std::vector<double> userDefRkbDiff;
            featureUi.tvdrkbDiffForWellPaths(&userDefRkbDiff);
            lasExporter.setRkbDiffs(wellNames, userDefRkbDiff);
        }

        lasExporter.writeToFolder(featureUi.exportFolder());

        // Remember the path to next time
        RiaApplication::instance()->setLastUsedDialogDirectory("WELL_LOGS_DIR", featureUi.exportFolder());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export To LAS Files...");
}

