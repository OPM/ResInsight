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

#include "RicSaveEclipseInputVisibleCellsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicExportFeatureImpl.h"

#include "RicSaveEclipseInputVisibleCellsUi.h"

#include "RifEclipseInputFileTools.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "Rim3dView.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>

CAF_CMD_SOURCE_INIT(RicSaveEclipseInputVisibleCellsFeature, "RicSaveEclipseInputVisibleCellsFeature");
CAF_CMD_SOURCE_INIT(RicSaveEclipseInputActiveVisibleCellsFeature, "RicSaveEclipseInputActiveVisibleCellsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void executeCommand(RimEclipseView* view)
{
    if (!view) return;

    RicSaveEclipseInputVisibleCellsUi exportSettings;
    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export FLUXNUM/MULTNUM", "");
    RicExportFeatureImpl::configureForExport(&propertyDialog);

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        std::vector<double> values;
        cvf::UByteArray visibleCells;
        view->calculateCurrentTotalCellVisibility(&visibleCells, view->currentTimeStep());
        RigActiveCellInfo* activeCellInfo = view->eclipseCase()->eclipseCaseData()->activeCellInfo(RiaDefines::MATRIX_MODEL);
        values.resize(visibleCells.size());
        for (size_t i = 0; i < visibleCells.size(); ++i)
        {
            if (activeCellInfo->isActive(i))
            {
                if (visibleCells[i])
                {
                    values[i] = static_cast<double>(exportSettings.visibleActiveCellsValue);
                }
                else
                {
                    values[i] = static_cast<double>(exportSettings.hiddenActiveCellsValue);
                }
            }
            else
            {
                values[i] = static_cast<double>(exportSettings.inactiveCellsValue);
            }
        }
        QFile exportFile(exportSettings.exportFilename);
        if (!exportFile.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            RiaLogging::error(QString("saveEclipseInputVisibleCells: Unable to open file '%1' for writing.").arg(exportSettings.exportFilename));
            return;
        }
        RifEclipseInputFileTools::writeDataToTextFile(&exportFile, exportSettings.exportKeyword().text(), values);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSaveEclipseInputVisibleCellsFeature::isCommandEnabled()
{
    return selectedView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputVisibleCellsFeature::onActionTriggered(bool isChecked)
{
    RimEclipseView* view = RicSaveEclipseInputVisibleCellsFeature::selectedView();
    executeCommand(view);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputVisibleCellsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Visible Cells as FLUXNUM/MULTNUM");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicSaveEclipseInputVisibleCellsFeature::selectedView() const
{
    RimEclipseView* view = dynamic_cast<RimEclipseView*>(caf::SelectionManager::instance()->selectedItem());
    if (view)
    {
        return view;
    }

    RimEclipseCellColors* cellResultItem = dynamic_cast<RimEclipseCellColors*>(caf::SelectionManager::instance()->selectedItem());
    if (cellResultItem)
    {
        cellResultItem->firstAncestorOrThisOfType(view);
    }

    return view;
}







//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicSaveEclipseInputActiveVisibleCellsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputActiveVisibleCellsFeature::onActionTriggered(bool isChecked)
{
    RimEclipseView* view = RicSaveEclipseInputActiveVisibleCellsFeature::getEclipseActiveView();
    executeCommand(view);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicSaveEclipseInputActiveVisibleCellsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Visible Cells as FLUXNUM/MULTNUM");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicSaveEclipseInputActiveVisibleCellsFeature::getEclipseActiveView()
{
    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();

    return dynamic_cast<RimEclipseView*>(activeView);
}

