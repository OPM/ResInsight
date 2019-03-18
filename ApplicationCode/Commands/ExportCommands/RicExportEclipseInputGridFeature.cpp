/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RicExportEclipseInputGridFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicExportFeatureImpl.h"
#include "RicExportEclipseInputGridUi.h"

#include "RifReaderEclipseOutput.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "Rim3dView.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QDir>

CAF_CMD_SOURCE_INIT(RicExportEclipseInputGridFeature, "RicExportEclipseInputGridFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseInputGridFeature::openDialogAndExecuteCommand(RimEclipseView* view)
{
    if (!view) return;

    RigEclipseCaseData* caseData = view->eclipseCase()->eclipseCaseData();

    RicExportEclipseInputGridUi exportSettings(caseData);
    caf::PdmUiPropertyViewDialog propertyDialog(Riu3DMainWindowTools::mainWindowWidget(), &exportSettings, "Export Input Grid", "");
    RicExportFeatureImpl::configureForExport(&propertyDialog);

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        executeCommand(view, exportSettings, "ExportInputGrid");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseInputGridFeature::executeCommand(RimEclipseView* view,
                                                      const RicExportEclipseInputGridUi& exportSettings,
                                                      const QString& logPrefix)
{
    int resultProgressPercentage = exportSettings.exportResults() ?
        std::min((int) exportSettings.exportMainKeywords().size(), 30) : 0;

    int gridProgressPercentage = 100 - resultProgressPercentage;
    caf::ProgressInfo progress(gridProgressPercentage + resultProgressPercentage, "Export Eclipse Data");
    
    cvf::Vec3st min, max;
    std::tie(min, max) = getVisibleCellRange(view);
    if (exportSettings.exportGrid())
    {
        auto task = progress.task("Export Input Grid", gridProgressPercentage);
        bool worked = RifReaderEclipseOutput::saveEclipseGrid(exportSettings.exportGridFilename(), view->eclipseCase()->eclipseCaseData(), &min, &max);
        if (!worked)
        {
            RiaLogging::error(
                QString("Unable to write grid to '%1'").arg(exportSettings.exportGridFilename));
        }
    }

    if (exportSettings.exportResults() != RicExportEclipseInputGridUi::EXPORT_NO_RESULTS)
    {
        auto                 task     = progress.task("Export Results", resultProgressPercentage);
        std::vector<QString> keywords = exportSettings.allSelectedKeywords();

        if (exportSettings.exportResults == RicExportEclipseInputGridUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT)
        {
            QFileInfo info(exportSettings.exportGridFilename());
            QDir dirPath = info.absoluteDir();
            QString fileWriteMode = "w";
            for (QString keyword : keywords)
            {
                QString fileName = dirPath.absoluteFilePath(keyword + ".GRDECL");
                bool worked = RifReaderEclipseOutput::saveEclipseResults(fileName,
                                                                         view->eclipseCase()->eclipseCaseData(),
                                                                         {keyword},
                                                                         fileWriteMode,
                                                                         &min,
                                                                         &max);
                if (!worked)
                {
                    RiaLogging::error(QString("Unable to write results to '%1'").arg(fileName));
                }
            }
        }
        else
        {
            QString fileWriteMode = "w";
            QString fileName = exportSettings.exportResultsFilename();
            if (exportSettings.exportResults() == RicExportEclipseInputGridUi::EXPORT_TO_GRID_FILE)
            {
                fileWriteMode = "a";
                fileName = exportSettings.exportGridFilename();
            }

            bool worked = RifReaderEclipseOutput::saveEclipseResults(fileName,
                view->eclipseCase()->eclipseCaseData(),
                keywords,
                fileWriteMode,
                &min,
                &max);

            if (!worked)
            {
                RiaLogging::error(QString("Unable to write results to '%1'").arg(fileName));
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st> RicExportEclipseInputGridFeature::getVisibleCellRange(RimEclipseView* view)
{
    cvf::UByteArray visibleCells;
    view->calculateCurrentTotalCellVisibility(&visibleCells, view->currentTimeStep());

    const RigMainGrid* mainGrid = view->eclipseCase()->mainGrid();
    cvf::Vec3st max = cvf::Vec3st::ZERO;
    cvf::Vec3st min = cvf::Vec3st(mainGrid->cellCountI() - 1,
                                  mainGrid->cellCountJ() - 1,
                                  mainGrid->cellCountK() - 1);

    size_t cellCount = mainGrid->cellCount();
    for (size_t index = 0; index < cellCount; ++index)
    {
        if (visibleCells[index])
        {
            cvf::Vec3st ijk;
            mainGrid->ijkFromCellIndex(index, &ijk[0], &ijk[1], &ijk[2]);
            for (int n = 0; n < 3; ++n)
            {
                min[n] = std::min(min[n], ijk[n]);
                max[n] = std::max(max[n], ijk[n]);
            }
        }
    }
    return std::make_pair(min, max);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportEclipseInputGridFeature::isCommandEnabled()
{
    return selectedView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseInputGridFeature::onActionTriggered(bool isChecked)
{
    RimEclipseView* view = RicExportEclipseInputGridFeature::selectedView();
    openDialogAndExecuteCommand(view);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseInputGridFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Visible Cells as Eclipse Input Grid");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicExportEclipseInputGridFeature::selectedView() const
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
