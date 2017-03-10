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

#include "RicAsciiExportWellLogPlotFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimWellLogPlot.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <vector>



CAF_CMD_SOURCE_INIT(RicAsciiExportWellLogPlotFeature, "RicAsciiExportWellLogPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportWellLogPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAsciiExportWellLogPlotFeature::onActionTriggered(bool isChecked)
{
    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    std::vector<RimWellLogPlot*> selectedWellLogPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedWellLogPlots);

    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("PLOT_ASCIIEXPORT_DIR", projectFolder);
    QString defaultFileName = defaultDir + "/" + QString("WellLogPlotExport");
    QString fileName = QFileDialog::getSaveFileName(NULL, "Select file for Well Log Plot Export", defaultFileName, "All files(*.*)");

    if (fileName.isEmpty()) return;
    bool isOk = writeAsciiExportForWellLogPlots(fileName, selectedWellLogPlots);

    if (!isOk)
    {
        QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + fileName);
    }
    
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAsciiExportWellLogPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Well Allocation Plot Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportWellLogPlotFeature::writeAsciiExportForWellLogPlots(const QString& fileName, const std::vector<RimWellLogPlot*>& selectedWellLogPlots)
{
    RiaLogging::info(QString("Writing ascii values for well allocation plot(s) to file: %1").arg(fileName));

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    caf::ProgressInfo pi(selectedWellLogPlots.size(), QString("Writing data to file %1").arg(fileName));
    size_t progress = 0;

    QTextStream out(&file);
    for (RimWellLogPlot* wellLogPlot : selectedWellLogPlots)
    {
        out << wellLogPlot->description();
        out << "\n";
        out << wellLogPlot->asciiDataForPlotExport();
        out << "\n\n";

        progress++;
        pi.setProgress(progress);
    }
    RiaLogging::info(QString("Competed writing ascii values for summary plot(s) to file %1").arg(fileName));
    return true;
}


