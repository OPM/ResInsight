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

#include "RicAsciiExportSummaryPlotFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RimSummaryPlot.h"
#include "RimSummaryPlotExportSettings.h"

#include "RiuMainWindow.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>
#include <QDebug>
#include <QFileInfo>
#include <QMessageBox>
#include <vector>



CAF_CMD_SOURCE_INIT(RicAsciiExportSummaryPlotFeature, "RicAsciiExportSummaryPlotFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportSummaryPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAsciiExportSummaryPlotFeature::onActionTriggered(bool isChecked)
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);

    RimSummaryPlotExportSettings exportSettings;

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("SUMMARYPLOT_ASCIIEXPORT_DIR", projectFolder);
    QString defaultFilename = QString("SummaryPlotExport"); 

    QString outputFileName = defaultDir + "/" + defaultFilename;
    exportSettings.fileName = outputFileName;

    caf::PdmUiPropertyViewDialog propertyDialog(RiuMainWindow::instance(), &exportSettings, "Export Summary Plots to Ascii", "");
    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("SUMMARYPLOT_ASCIIEXPORT_DIR", QFileInfo(exportSettings.fileName).absolutePath());

        bool isOk = writeAsciiExportForSummaryPlots(exportSettings.fileName(), selectedSummaryPlots);

        if (!isOk)
        {
            QMessageBox::critical(NULL, "File export", "Failed to exported current result to " + exportSettings.fileName);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAsciiExportSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Summary Plot Data");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportSummaryPlotFeature::writeAsciiExportForSummaryPlots(const QString& fileName, const std::vector<RimSummaryPlot*>& selectedSummaryPlots)
{
    RiaLogging::info(QString("Writing ascii values for summary plot(s) to file: %1").arg(fileName));

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    caf::ProgressInfo pi(selectedSummaryPlots.size(), QString("Writing data to file %1").arg(fileName));
    size_t progress = 0;

    QTextStream out(&file);
    for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
    {
        out << summaryPlot->description();
        out << summaryPlot->asciiDataForPlotExport();

        progress++;
        pi.setProgress(progress);
    }
    out << "\n";
    RiaLogging::info(QString("Competed writing ascii values for summary plot(s) to file %1").arg(fileName));
    return true;
}


