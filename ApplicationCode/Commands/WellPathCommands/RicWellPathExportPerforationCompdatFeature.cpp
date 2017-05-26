/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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

#include "RicWellPathExportPerforationCompdatFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimPerforationCollection.h"
#include "RimPerforationInterval.h"

#include "RiuMainWindow.h"

#include "RigWellLogExtractionTools.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigWellPath.h"
#include "RigWellPathIntersectionTools.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"

#include "cvfPlane.h"

#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

CAF_CMD_SOURCE_INIT(RicWellPathExportPerforationCompdatFeature, "RicWellPathExportPerforationCompdatFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellPathExportPerforationCompdatFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    if (objects.size() == 1) {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportPerforationCompdatFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> objects;
    caf::SelectionManager::instance()->objectsByType(&objects);

    CVF_ASSERT(objects.size() == 1);

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RimCaseAndFileExportSettings exportSettings;
    std::vector<RimCase*> cases;
    app->project()->allCases(cases);
    for (auto c : cases)
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(c);
        if (eclipseCase != nullptr)
        {
            exportSettings.caseToApply = eclipseCase;
            break;
        }
    }

    exportSettings.fileName = QDir(defaultDir).filePath("Perforations");

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Completion Data", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("COMPLETIONS", QFileInfo(exportSettings.fileName).absolutePath());

        exportCompdat(objects[0], exportSettings);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportPerforationCompdatFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Perforation Completion Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportPerforationCompdatFeature::exportCompdat(RimWellPath* wellPath, const RimCaseAndFileExportSettings& exportSettings)
{
    QFile exportFile(exportSettings.fileName());

    if (exportSettings.caseToApply() == nullptr)
    {
        RiaLogging::error("Export Completions Data: Cannot export completions data without specified eclipse case");
        return;
    }

    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error(QString("Export Completions Data: Could not open the file: %1").arg(exportSettings.fileName()));
        return;
    }

    // Generate data
    const RigEclipseCaseData* caseData =  exportSettings.caseToApply()->eclipseCaseData();
    const RigMainGrid* grid = caseData->mainGrid();


    QTextStream stream(&exportFile);
    RifEclipseOutputTableFormatter formatter(stream);

    std::vector<RifEclipseOutputTableColumn> header = {
               RifEclipseOutputTableColumn("Well"),
               RifEclipseOutputTableColumn("I"),
               RifEclipseOutputTableColumn("J"),
               RifEclipseOutputTableColumn("K1"),
               RifEclipseOutputTableColumn("K2"),
               RifEclipseOutputTableColumn("Status"),
               RifEclipseOutputTableColumn("SAT"),
               RifEclipseOutputTableColumn("TR"),
               RifEclipseOutputTableColumn("DIAM"),
               RifEclipseOutputTableColumn("KH"),
               RifEclipseOutputTableColumn("S"),
               RifEclipseOutputTableColumn("Df"),
               RifEclipseOutputTableColumn("DIR"),
               RifEclipseOutputTableColumn("r0")
    };

    formatter.keyword("COMPDAT");
    formatter.header(header);

    for (const RimPerforationInterval* interval : wellPath->perforationIntervalCollection()->m_perforations())
    {
        generateCompdatForPerforation(formatter, caseData, wellPath, interval);
    }

    formatter.tableCompleted();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellPathExportPerforationCompdatFeature::generateCompdatForPerforation(RifEclipseOutputTableFormatter& formatter, const RigEclipseCaseData* caseData, RimWellPath* wellPath, const RimPerforationInterval* interval)
{
    std::vector<cvf::Vec3d> perforationPoints = wellPath->wellPathGeometry()->clippedPointSubset(interval->startMD(), interval->endMD());
    std::vector<WellPathCellIntersectionInfo> intersectedCells = RigWellPathIntersectionTools::findCellsIntersectedByPath(caseData, perforationPoints);

    formatter.comment(QString("Perforation interval from %1 to %2").arg(interval->startMD()).arg(interval->endMD()));
    for (auto& cell : intersectedCells)
    {
        size_t i, j, k;
        caseData->mainGrid()->ijkFromCellIndex(cell.cellIndex, &i, &j, &k);
        formatter.add(wellPath->name());
        formatter.addZeroBasedCellIndex(i).addZeroBasedCellIndex(j).addZeroBasedCellIndex(k).addZeroBasedCellIndex(k);
        formatter.add("'OPEN'").add("1*").add("1*");
        formatter.add(interval->radius());
        formatter.add("1*").add("1*").add("1*");
        switch (cell.direction)
        {
        case POS_I:
        case NEG_I:
            formatter.add("'X'");
            break;
        case POS_J:
        case NEG_J:
            formatter.add("'Y'");
            break;
        case POS_K:
        case NEG_K:
            formatter.add("'Z'");
            break;
        }
        formatter.add("1*");
        formatter.rowCompleted();
    }
}
