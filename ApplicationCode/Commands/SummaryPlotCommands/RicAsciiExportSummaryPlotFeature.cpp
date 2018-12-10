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

#include "RicResampleDialog.h"

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

    std::vector<RimSummaryPlot*> selectedSummaryPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedSummaryPlots);
    QString defaultDir = defaultExportDir();

    // Ask user about resampling
    auto result = RicResampleDialog::openDialog();
    if (!result.ok) return;

    if (selectedSummaryPlots.size() == 1)
    {
        RimSummaryPlot* summaryPlot = selectedSummaryPlots.at(0);
        QString fileName = getFileNameFromUserDialog(summaryPlot->description(), defaultDir);
        if (fileName.isEmpty()) return;

        caf::ProgressInfo pi(selectedSummaryPlots.size(), QString("Exporting plot data to ASCII"));
        size_t progress = 0;

        RicAsciiExportSummaryPlotFeature::exportAsciiForSummaryPlot(fileName, summaryPlot, result.period);

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

        caf::ProgressInfo pi(selectedSummaryPlots.size(), QString("Exporting plot data to ASCII"));
        size_t progress = 0;

        RiaLogging::info(QString("Writing to directory %1").arg(saveDir));
        for (RimSummaryPlot* summaryPlot : selectedSummaryPlots)
        {
            QString fileName = saveDir + "/" + caf::Utils::makeValidFileBasename(summaryPlot->description()) + ".ascii";
            RicAsciiExportSummaryPlotFeature::exportAsciiForSummaryPlot(fileName, summaryPlot, result.period); 
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
QString RicAsciiExportSummaryPlotFeature::defaultExportDir()
{
    return RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder("PLOT_ASCIIEXPORT_DIR");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QString RicAsciiExportSummaryPlotFeature::getFileNameFromUserDialog(const QString& fileNameCandidate, const QString& defaultDir)
{
    QString defaultFileName = defaultDir + "/" + caf::Utils::makeValidFileBasename(fileNameCandidate) + ".ascii";
    QString fileName = QFileDialog::getSaveFileName(nullptr, "Select File for Summary Plot Export", defaultFileName, "Text File(*.ascii);;All files(*.*)");
    return fileName;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportSummaryPlotFeature::exportTextToFile(const QString& fileName, const QString& text)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    RiaLogging::info(QString("Writing values for summary plot(s) to file: %1").arg(fileName));

    QTextStream out(&file);

    out << text;

    RiaLogging::info(QString("Competed writing values for summary plot(s) to file %1").arg(fileName));

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportSummaryPlotFeature::exportAsciiForSummaryPlot(const QString& fileName,
                                                                 const RimSummaryPlot* summaryPlot,
                                                                 DateTimePeriod resamplingPeriod)
{
    QString text = summaryPlot->description();
    text.append(summaryPlot->asciiDataForPlotExport(resamplingPeriod));
    text.append("\n\n");

    return exportTextToFile(fileName, text);
}

