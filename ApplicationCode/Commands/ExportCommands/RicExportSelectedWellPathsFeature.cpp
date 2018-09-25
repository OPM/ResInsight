/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2016-     Statoil ASA
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

#include "RicExportSelectedWellPathsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicExportWellPathsUi.h"

#include "RigWellPath.h"

#include "RimWellPath.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimDialogData.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManagerTools.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QFileInfo>
#include <QFileDialog>
#include <QMessageBox>

#include <memory>


CAF_CMD_SOURCE_INIT(RicExportSelectedWellPathsFeature, "RicExportSelectedWellPathsFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::exportWellPath(const RimWellPath* wellPath, double mdStepSize, const QString& folder)
{
    auto geom = wellPath->wellPathGeometry();
    double currMd = geom->measureDepths().front() - mdStepSize;
    double endMd = geom->measureDepths().back();

    auto fileName = wellPath->name() + ".dev";
    auto filePtr = openFileForExport(folder, fileName);
    QTextStream stream(filePtr.get());
    stream.setRealNumberNotation(QTextStream::FixedNotation);

    stream << "WELLNAME: '" << wellPath->name() << "'" << endl;

    while (currMd < endMd)
    {
        currMd += mdStepSize;
        if (currMd > endMd) currMd = endMd;

        auto pt = geom->interpolatedPointAlongWellPath(currMd);
        double tvd = -pt.z();

        // Write to file
        stream << pt.x() << " " << pt.y() << " " << tvd << " " << currMd << endl;
    }
    
    filePtr->close();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QFilePtr RicExportSelectedWellPathsFeature::openFileForExport(const QString& folderName, const QString& fileName)
{
    QDir exportFolder = QDir(folderName);
    if (!exportFolder.exists())
    {
        bool createdPath = exportFolder.mkpath(".");
        if (createdPath)
            RiaLogging::info("Created export folder " + folderName);
    }

    QString  filePath = exportFolder.filePath(fileName);
    QFilePtr exportFile(new QFile(filePath));
    if (!exportFile->open(QIODevice::WriteOnly))
    {
        auto errorMessage = QString("Export Well Path: Could not open the file: %1").arg(filePath);
        RiaLogging::error(errorMessage);
    }
    return exportFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::handleAction(const std::vector<RimWellPath*>& wellPaths)
{
    if (!wellPaths.empty())
    {
        auto dialogData = openDialog();
        if (dialogData)
        {
            auto folder = dialogData->exportFolder();
            auto mdStepSize = dialogData->mdStepSize();
            if (folder.isEmpty())
            {
                return;
            }

            for (auto wellPath : wellPaths)
            {
                exportWellPath(wellPath, mdStepSize, folder);
            }

        }

    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportSelectedWellPathsFeature::isCommandEnabled()
{
    std::vector<RimWellPath*> wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    return !wellPaths.empty();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = caf::selectedObjectsByTypeStrict<RimWellPath*>();

    handleAction(wellPaths);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Selected Well Paths");
    actionToSetup->setIcon(QIcon(":/WellLogCurve16x16.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RicExportWellPathsUi* RicExportSelectedWellPathsFeature::openDialog()
{
    RiaApplication* app = RiaApplication::instance();
    RimProject* proj = app->project();

    QString startPath = app->lastUsedDialogDirectory("WELL_LOGS_DIR");
    if (startPath.isEmpty())
    {
        QFileInfo fi(proj->fileName());
        startPath = fi.absolutePath();
    }

    RicExportWellPathsUi* featureUi = app->project()->dialogData()->wellPathsExportData();

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, featureUi, "Export Well Paths", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(600, 60));

    if (propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty())
    {
        auto dialogData = app->project()->dialogData()->wellPathsExportData();
        app->setLastUsedDialogDirectory("WELL_LOGS_DIR", dialogData->exportFolder());
        return dialogData;
    }
    return nullptr;
}
