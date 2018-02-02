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
#include "cafUtils.h"
#include "cvfAssert.h"

#include <QAction>
#include <QDebug>
#include <QFileDialog>
#include <QFileInfo>



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
    this->disableModelChangeContribution();

    RiaApplication* app = RiaApplication::instance();
    QString projectFolder = app->currentProjectPath();

    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT(project);

    std::vector<RimWellLogPlot*> selectedWellLogPlots;
    caf::SelectionManager::instance()->objectsByType(&selectedWellLogPlots);
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("PLOT_ASCIIEXPORT_DIR", projectFolder);

    caf::ProgressInfo pi(selectedWellLogPlots.size(), QString("Exporting plot data to ASCII"));
    size_t progress = 0;

    if (selectedWellLogPlots.size() == 1)
    {
        RimWellLogPlot* wellLogPlot = selectedWellLogPlots.at(0);
        QString defaultFileName = defaultDir + "/" + caf::Utils::makeValidFileBasename((wellLogPlot->description())) + ".ascii";
        QString fileName = QFileDialog::getSaveFileName(nullptr, "Select File for Plot Data Export", defaultFileName, "Text File(*.ascii);;All files(*.*)");
        if (fileName.isEmpty()) return;
        RicAsciiExportWellLogPlotFeature::exportAsciiForWellLogPlot(fileName, wellLogPlot);

        progress++;
        pi.setProgress(progress);
    }
    else if (selectedWellLogPlots.size() > 1)
    {
        std::vector<QString> fileNames;
        for (RimWellLogPlot* wellLogPlot : selectedWellLogPlots)
        {
            QString fileName = caf::Utils::makeValidFileBasename(wellLogPlot->description()) + ".ascii";
            fileNames.push_back(fileName);
        }

        QString saveDir;
        bool writeFiles = caf::Utils::getSaveDirectoryAndCheckOverwriteFiles(defaultDir, fileNames, &saveDir);
        if (!writeFiles) return;

        RiaLogging::info(QString("Writing to directory %!").arg(saveDir));
        for (RimWellLogPlot* wellLogPlot : selectedWellLogPlots)
        {
            QString fileName = saveDir + "/" + caf::Utils::makeValidFileBasename(wellLogPlot->description()) + ".ascii";
            RicAsciiExportWellLogPlotFeature::exportAsciiForWellLogPlot(fileName, wellLogPlot);

            progress++;
            pi.setProgress(progress);
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicAsciiExportWellLogPlotFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Plot Data to Text File");
    actionToSetup->setIcon(QIcon(":/Save.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicAsciiExportWellLogPlotFeature::exportAsciiForWellLogPlot(const QString& fileName, const RimWellLogPlot* wellLogPlot)
{
    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
    {
        return false;
    }

    RiaLogging::info(QString("Writing values for plot(s) to file: %1").arg(fileName));
    
    QTextStream out(&file);

    out << wellLogPlot->description();
    out << "\n";
    out << wellLogPlot->asciiDataForPlotExport();
    out << "\n\n";
    
    RiaLogging::info(QString("Competed writing values for plot(s) to file %1").arg(fileName));

    return true;
}


