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
#include "cafUtils.h"

#include "cvfAssert.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>




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
    this->disableModelChangeContribution();

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("PLOT_ASCIIEXPORT_DIR", projectFolder);

    caf::ProgressInfo pi(selectedSummaryPlots.size(), QString("Exporting plot data to ASCII"));
    size_t progress = 0;

    if (selectedSummaryPlots.size() == 1)
    {
        RimSummaryPlot* summaryPlot = selectedSummaryPlots.at(0);
        QString defaultFileName = defaultDir + "/" + caf::Utils::makeValidFileBasename((summaryPlot->description())) + ".ascii";
        QString fileName = QFileDialog::getSaveFileName(nullptr, "Select File for Summary Plot Export", defaultFileName, "Text File(*.ascii);;All files(*.*)");
        if (fileName.isEmpty()) return;
        RicAsciiExportSummaryPlotFeature::exportAsciiForSummaryPlot(fileName, summaryPlot); 

        progress++;
        pi.setProgress(progress);
    }
    else if (selectedSummaryPlots.size() > 1)
    {
        std::vector<QString> fileNames;
        for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
        {
            QString fileName = caf::Utils::makeValidFileBasename(summaryPlot->description()) + ".ascii";
            fileNames.push_back(fileName);
        }

        QString saveDir;
        bool writeFiles = caf::Utils::getSaveDirectoryAndCheckOverwriteFiles(defaultDir, fileNames, &saveDir);
        if (!writeFiles) return;

        RiaLogging::info(QString("Writing to directory %1").arg(saveDir));
        for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
        {
            QString fileName = saveDir + "/" + caf::Utils::makeValidFileBasename(summaryPlot->description()) + ".ascii";
            RicAsciiExportSummaryPlotFeature::exportAsciiForSummaryPlot(fileName, summaryPlot); 
            progress++;
            pi.setProgress(progress);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAsciiExportSummaryPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Plot Data to Text File");
    actionToSetup->setIcon(QIcon(":/Save.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportSummaryPlotFeature::exportAsciiForSummaryPlot(const QString& fileName, const RimSummaryPlot* summaryPlot)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    RiaLogging::info(QString("Writing values for summary plot(s) to file: %1").arg(fileName));

    QTextStream out(&file);
    
    out << summaryPlot->description();
    out << summaryPlot->asciiDataForPlotExport();
    out << "\n\n";

    RiaLogging::info(QString("Competed writing values for summary plot(s) to file %1").arg(fileName));

    return true;
}

