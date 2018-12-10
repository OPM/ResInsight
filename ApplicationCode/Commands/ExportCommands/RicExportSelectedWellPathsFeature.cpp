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

#include "RifEclipseDataTableFormatter.h"

#include "RigWellPath.h"

#include "RimWellPath.h"
#include "RimModeledWellPath.h"
#include "RimWellPathGeometryDef.h"
#include "RimProject.h"
#include "RimDialogData.h"

#include "cafSelectionManagerTools.h"
#include "cafPdmUiPropertyViewDialog.h"

#include <QAction>
#include <QFileDialog>

#include <cafUtils.h>

#include <memory>


CAF_CMD_SOURCE_INIT(RicExportSelectedWellPathsFeature, "RicExportSelectedWellPathsFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::exportWellPath(const RimWellPath* wellPath,
                                                       double mdStepSize,
                                                       const QString& folder,
                                                       bool writeProjectInfo)
{
    auto fileName = wellPath->name() + ".dev";
    auto filePtr = openFileForExport(folder, fileName);
    auto stream = createOutputFileStream(*filePtr);

    writeWellPathGeometryToStream(*stream, wellPath, wellPath->name(), mdStepSize, writeProjectInfo);
    filePtr->close();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportSelectedWellPathsFeature::writeWellPathGeometryToStream(QTextStream& stream,
                                                                      const RimWellPath* wellPath,
                                                                      const QString& exportName,
                                                                      double mdStepSize,
                                                                      bool writeProjectInfo)
{
    auto wellPathGeom = wellPath->wellPathGeometry();
    if (!wellPathGeom) return;

    double currMd = wellPathGeom->measureDepths().front() - mdStepSize;
    double endMd = wellPathGeom->measureDepths().back();

    RifEclipseDataTableFormatter formatter(stream);
    formatter.setCommentPrefix("#");
    formatter.setTableRowPrependText("  ");

    if (writeProjectInfo)
    {
        formatter.comment("Project: " + RiaApplication::instance()->project()->fileName);
        stream << endl;
    }

    bool useMdRkb = false;
    double rkb = 0.0;
    {
        const RimModeledWellPath* modeledWellPath = dynamic_cast<const RimModeledWellPath*>(wellPath);
        if (modeledWellPath)
        {
            useMdRkb = true;
            rkb = modeledWellPath->geometryDefinition()->mdrkbAtFirstTarget();
        }
    }

    stream << "WELLNAME: '" << caf::Utils::makeValidFileBasename(exportName) << "'" << endl;

    auto numberFormat = RifEclipseOutputTableDoubleFormatting(RIF_FLOAT, 2);
    formatter.header(
    {
        {"X", numberFormat, RIGHT},
        {"Y", numberFormat, RIGHT},
        {"TVDMSL", numberFormat, RIGHT},
        {useMdRkb ? "MDRKB" : "MDMSL", numberFormat, RIGHT}
    });

    while (currMd < endMd)
    {
        currMd += mdStepSize;
        if (currMd > endMd) currMd = endMd;

        auto pt = wellPathGeom->interpolatedPointAlongWellPath(currMd);
        double tvd = -pt.z();

        // Write to file
        formatter.add(pt.x());
        formatter.add(pt.y());
        formatter.add(tvd);
        formatter.add(currMd + rkb);
        formatter.rowCompleted("");
    }
    formatter.tableCompleted("", false);

    stream << -999 << endl << endl;
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
    if (!exportFile->open(QIODevice::WriteOnly | QIODevice::Text))
    {
        auto errorMessage = QString("Export Well Path: Could not open the file: %1").arg(filePath);
        RiaLogging::error(errorMessage);
    }
    return exportFile;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
QTextStreamPtr RicExportSelectedWellPathsFeature::createOutputFileStream(QFile& file)
{
    auto stream = QTextStreamPtr(new QTextStream(&file));
    stream->setRealNumberNotation(QTextStream::FixedNotation);
    stream->setRealNumberPrecision(2);
    return stream;
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

    QString startPath = app->lastUsedDialogDirectoryWithFallbackToProjectFolder("WELL_PATH_EXPORT_DIR");
    if (startPath.isEmpty())
    {
        QFileInfo fi(proj->fileName());
        startPath = fi.absolutePath();
    }

    RicExportWellPathsUi* featureUi = app->project()->dialogData()->wellPathsExportData();
    if (featureUi->exportFolder().isEmpty())
    {
        featureUi->setExportFolder(startPath);
    }

    caf::PdmUiPropertyViewDialog propertyDialog(nullptr, featureUi, "Export Well Paths", "", QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    propertyDialog.resize(QSize(600, 60));

    if (propertyDialog.exec() == QDialog::Accepted && !featureUi->exportFolder().isEmpty())
    {
        auto dialogData = app->project()->dialogData()->wellPathsExportData();
        app->setLastUsedDialogDirectory("WELL_PATH_EXPORT_DIR", dialogData->exportFolder());
        return dialogData;
    }
    return nullptr;
}
