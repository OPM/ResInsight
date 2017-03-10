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
#include "cafUtils.h"



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
    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("PLOT_ASCIIEXPORT_DIR", projectFolder);


    caf::ProgressInfo pi(selectedSummaryPlots.size(), QString("Exporting to csv"));
    size_t progress = 0;


    bool isOk = false;
    if (selectedSummaryPlots.size() == 1)
    {
        RimSummaryPlot* summaryPlot = selectedSummaryPlots.at(0);
        QString defaultFileName = defaultDir + "/" + caf::Utils::makeValidFileBasename((summaryPlot->description())) + ".csv";
        QString fileName = QFileDialog::getSaveFileName(NULL, "Select file for Summary Plot Export", defaultFileName, "All files(*.*)");
        if (fileName.isEmpty()) return;
        isOk = writeAsciiExportForSummaryPlots(fileName, summaryPlot); 

        progress++;
        pi.setProgress(progress);
    }
    else if (selectedSummaryPlots.size() > 1)
    {
        std::vector<QString> fileNames;
        for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
        {
            QString fileName = caf::Utils::makeValidFileBasename(summaryPlot->description()) + ".csv";
            fileNames.push_back(fileName);
        }

        QString saveDir;
        bool writeFiles = caf::Utils::getSaveDirectoryAndCheckOverwriteFiles(defaultDir, fileNames, &saveDir);
        if (!writeFiles) return;

        RiaLogging::debug(QString("Writing to directory %!").arg(saveDir));
        for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
        {
            QString fileName = saveDir + "/" + caf::Utils::makeValidFileBasename(summaryPlot->description()) + ".csv";
            isOk = writeAsciiExportForSummaryPlots(fileName, summaryPlot); 
            progress++;
            pi.setProgress(progress);
        }
    }

    if (!isOk)
    {
        QMessageBox::critical(NULL, "File export", "Failed to export summary plots to csv");
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
bool RicAsciiExportSummaryPlotFeature::writeAsciiExportForSummaryPlots(const QString& fileName, const RimSummaryPlot* summaryPlot)
{
    RiaLogging::info(QString("Writing ascii values for summary plot(s) to file: %1").arg(fileName));

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }


    QTextStream out(&file);
    out << summaryPlot->description();
    out << summaryPlot->asciiDataForPlotExport();
    out << "\n\n";

    RiaLogging::info(QString("Competed writing ascii values for summary plot(s) to file %1").arg(fileName));
    return true;
}


