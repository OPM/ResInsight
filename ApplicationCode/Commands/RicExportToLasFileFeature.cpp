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
#include "RicExportToLasFileResampleUi.h"
#include "RigLasFileExporter.h"
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
    return selectedWellLogCurves().size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportToLasFileFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellLogCurve*> curves = selectedWellLogCurves();
    if (curves.size() == 0) return;

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("WELL_LOGS_DIR", projectFolder);

    RicExportToLasFileResampleUi featureUi;
    featureUi.exportFolder = defaultDir;
    caf::PdmUiPropertyViewDialog propertyDialog(NULL, &featureUi, "Export Curve Data to LAS file(s)", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(350, 200));
    
    if (propertyDialog.exec() == QDialog::Accepted &&
        !featureUi.exportFolder().isEmpty())
    {
        RigLasFileExporter lasExporter(curves);

        if (featureUi.activateResample)
        {
            lasExporter.setResamplingInterval(featureUi.resampleInterval());
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

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::vector<RimWellLogCurve*> RicExportToLasFileFeature::selectedWellLogCurves() const
{
    std::set<RimWellLogCurve*> curveSet;

    {
        std::vector<caf::PdmUiItem*> selectedItems;
        caf::SelectionManager::instance()->selectedItems(selectedItems);

        for (auto selectedItem : selectedItems)
        {
            caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(selectedItem);
            if (objHandle)
            {
                std::vector<RimWellLogCurve*> childCurves;
                objHandle->descendantsIncludingThisOfType(childCurves);

                for (auto curve : childCurves)
                {
                    curveSet.insert(curve);
                }
            }
        }
    }

    std::vector<RimWellLogCurve*> allCurves;
    for (auto curve : curveSet)
    {
        allCurves.push_back(curve);
    }

    return allCurves;
}

