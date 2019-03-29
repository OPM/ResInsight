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

#include "RicExportEclipseSectorModelFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicExportFeatureImpl.h"
#include "RicExportEclipseSectorModelUi.h"

#include "RifEclipseInputFileTools.h"
#include "RifReaderEclipseOutput.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimFaultInView.h"
#include "RimFaultInViewCollection.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"

#include <QAction>
#include <QFileInfo>
#include <QDir>

CAF_CMD_SOURCE_INIT(RicExportEclipseSectorModelFeature, "RicExportEclipseInputGridFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::openDialogAndExecuteCommand(RimEclipseView* view)
{
    if (!view) return;

    RigEclipseCaseData* caseData = view->eclipseCase()->eclipseCaseData();

    RicExportEclipseSectorModelUi exportSettings(caseData);
    caf::PdmUiPropertyViewDialog propertyDialog(Riu3DMainWindowTools::mainWindowWidget(), &exportSettings, "Export Eclipse Sector Model", "");
    RicExportFeatureImpl::configureForExport(&propertyDialog);

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        executeCommand(view, exportSettings, "ExportInputGrid");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::executeCommand(RimEclipseView* view,
                                                      const RicExportEclipseSectorModelUi& exportSettings,
                                                      const QString& logPrefix)
{
    int resultProgressPercentage = exportSettings.exportResults() ?
        std::min((int) exportSettings.exportMainKeywords().size(), 20) : 0;

    int faultsProgressPercentage = exportSettings.exportFaults() ? 10 : 0;

    int gridProgressPercentage = 100 - resultProgressPercentage - faultsProgressPercentage;
    caf::ProgressInfo progress(gridProgressPercentage + resultProgressPercentage + faultsProgressPercentage,
                               "Export Eclipse Sector Model");

    cvf::Vec3st refinement(exportSettings.cellCountI(), exportSettings.cellCountJ(), exportSettings.cellCountK());

    CVF_ASSERT(refinement.x() > 0u && refinement.y() > 0u && refinement.z() > 0u);

    cvf::UByteArray cellVisibility;
    view->calculateCurrentTotalCellVisibility(&cellVisibility, view->currentTimeStep());

    cvf::Vec3st min, max;
    std::tie(min, max) = getVisibleCellRange(view, cellVisibility);
    if (exportSettings.exportGrid())
    {
        const cvf::UByteArray* cellVisibilityForActnum = exportSettings.makeInvisibleCellsInactive() ? &cellVisibility : nullptr;
        auto task = progress.task("Export Grid", gridProgressPercentage);

        bool worked = RifEclipseInputFileTools::exportGrid(exportSettings.exportGridFilename(),
                                                           view->eclipseCase()->eclipseCaseData(),
                                                           cellVisibilityForActnum,
                                                           min,
                                                           max,
                                                           refinement);

        if (!worked)
        {
            RiaLogging::error(
                QString("Unable to write grid to '%1'").arg(exportSettings.exportGridFilename));
        }
        else
        {
            if (view->eclipseCase()->eclipseCaseData()->gridCount() > 1u)
            {
                RiaLogging::warning("Grid has LGRs but ResInsight only supports exporting the Main Grid");
            }

            QFileInfo info(exportSettings.exportGridFilename());
            RiaApplication::instance()->setLastUsedDialogDirectory("EXPORT_INPUT_GRID", info.absolutePath());
        }
    }

    if (exportSettings.exportResults() != RicExportEclipseSectorModelUi::EXPORT_NO_RESULTS)
    {
        auto                 task     = progress.task("Export Properties", resultProgressPercentage);
        std::vector<QString> keywords = exportSettings.allSelectedKeywords();

        if (exportSettings.exportResults == RicExportEclipseSectorModelUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT)
        {
            QFileInfo info(exportSettings.exportGridFilename());
            QDir dirPath = info.absoluteDir();
            QString fileWriteMode = "w";
            for (QString keyword : keywords)
            {
                QString fileName = dirPath.absoluteFilePath(keyword + ".GRDECL");
                bool worked = RifEclipseInputFileTools::exportKeywords(fileName,
                                                                       view->eclipseCase()->eclipseCaseData(),
                                                                       {keyword},
                                                                       fileWriteMode,
                                                                       min,
                                                                       max,
                                                                       refinement);
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
            if (exportSettings.exportResults() == RicExportEclipseSectorModelUi::EXPORT_TO_GRID_FILE)
            {
                fileWriteMode = "a";
                fileName = exportSettings.exportGridFilename();
            }

            bool worked = RifEclipseInputFileTools::exportKeywords(fileName,
                view->eclipseCase()->eclipseCaseData(),
                keywords,
                fileWriteMode,
                min,
                max,
                refinement);

            if (!worked)
            {
                RiaLogging::error(QString("Unable to write results to '%1'").arg(fileName));
            }
        }
    }

    if (exportSettings.exportFaults() != RicExportEclipseSectorModelUi::EXPORT_NO_RESULTS)
    {
        auto task = progress.task("Export Faults", faultsProgressPercentage);
        if (exportSettings.exportFaults == RicExportEclipseSectorModelUi::EXPORT_TO_SEPARATE_FILE_PER_RESULT)
        {
            QFileInfo info(exportSettings.exportGridFilename());
            QDir      dirPath       = info.absoluteDir();

            for (auto faultInView : view->faultCollection()->faults())
            {
                auto rigFault = faultInView->faultGeometry();
                QString fileName = QString("%1.GRDECL").arg(rigFault->name());
                RifEclipseInputFileTools::saveFault(
                    fileName, view->eclipseCase()->mainGrid(), rigFault->faultFaces(), rigFault->name(), min, max, refinement);
            }
        }
        else
        {            
            QString fileName      = exportSettings.exportFaultsFilename();
            QIODevice::OpenMode openFlag = QIODevice::Truncate;
            if (exportSettings.exportResults() == RicExportEclipseSectorModelUi::EXPORT_TO_GRID_FILE)
            {
                openFlag = QIODevice::Append;
                fileName = exportSettings.exportGridFilename();
            }
            QFile exportFile(fileName);

            if (!exportFile.open(QIODevice::Text | QIODevice::WriteOnly | openFlag))
            {
                RiaLogging::error("Could not open the file : " + fileName);
            }

            QTextStream stream(&exportFile);
            RifEclipseInputFileTools::saveFaults(stream, view->eclipseCase()->mainGrid(), min, max, refinement);
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Vec3st, cvf::Vec3st> RicExportEclipseSectorModelFeature::getVisibleCellRange(RimEclipseView* view, const cvf::UByteArray& cellVisibillity)
{

    const RigMainGrid* mainGrid = view->eclipseCase()->mainGrid();
    cvf::Vec3st max = cvf::Vec3st::ZERO;
    cvf::Vec3st min = cvf::Vec3st(mainGrid->cellCountI() - 1,
                                  mainGrid->cellCountJ() - 1,
                                  mainGrid->cellCountK() - 1);

    size_t cellCount = mainGrid->cellCount();
    for (size_t index = 0; index < cellCount; ++index)
    {
        if (cellVisibillity[index])
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
bool RicExportEclipseSectorModelFeature::isCommandEnabled()
{
    return selectedView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::onActionTriggered(bool isChecked)
{
    RimEclipseView* view = RicExportEclipseSectorModelFeature::selectedView();
    openDialogAndExecuteCommand(view);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportEclipseSectorModelFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Visible Cells as Eclipse Sector Model");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseView* RicExportEclipseSectorModelFeature::selectedView() const
{
    RimEclipseView* view = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimEclipseView>();
    if (view)
    {
        return view;
    }

    Rim3dView* activeView = RiaApplication::instance()->activeReservoirView();
    return dynamic_cast<RimEclipseView*>(activeView);
}
