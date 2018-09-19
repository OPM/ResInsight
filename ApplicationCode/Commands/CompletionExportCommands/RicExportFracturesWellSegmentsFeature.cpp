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

#include "RicExportFracturesWellSegmentsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicCaseAndFileExportSettingsUi.h"
#include "RicExportFeatureImpl.h"
#include "RicMultiSegmentWellExportInfo.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"

#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathFracture.h"
#include "RimWellPathFractureCollection.h"

#include "RifEclipseDataTableFormatter.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafUtils.h"

#include <QAction>
#include <QDir>


CAF_CMD_SOURCE_INIT(RicExportFracturesWellSegmentsFeature, "RicExportFracturesWellSegmentsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportFracturesWellSegmentsFeature::exportWellSegments(const RimWellPath* wellPath, const std::vector<RimWellPathFracture*>& fractures, const RicCaseAndFileExportSettingsUi& settings)
{
    if (settings.caseToApply() == nullptr)
    {
        RiaLogging::error("Export Fracture Well Segments: Cannot export completions data without specified eclipse case");
        return;
    }

    QString fileName = QString("%1-Fracture-Welsegs").arg(settings.caseToApply()->caseUserDescription());
    fileName = caf::Utils::makeValidFileBasename(fileName);

    QDir exportFolder(settings.folder());
    if (!exportFolder.exists())
    {
        bool createdPath = exportFolder.mkpath(".");
        if (createdPath)
            RiaLogging::info("Export Fracture Well Segments: Created export folder " + settings.folder());
        else
            RiaLogging::error("Export Fracture Well Segments: Selected output folder does not exist, and could not be created.");
    }

    QString filePath = exportFolder.filePath(fileName);
    QFile   exportFile(filePath);
    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error(QString("Export Fracture Well Segments: Could not open the file: %1").arg(filePath));
        return;
    }

    RicMswExportInfo exportInfo = RicWellPathExportCompletionDataFeatureImpl::generateFracturesMswExportInfo(settings.caseToApply, wellPath, fractures);

    QTextStream stream(&exportFile);
    RifEclipseDataTableFormatter formatter(stream);
    RicWellPathExportCompletionDataFeatureImpl::generateWelsegsTable(formatter, exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateCompsegTables(formatter, exportInfo);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFracturesWellSegmentsFeature::onActionTriggered(bool isChecked)
{
    RimWellPath*                   wellPath   = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
    RimWellPathFracture*           fracture   = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPathFracture>();
    RimWellPathFractureCollection* collection = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPathFractureCollection>();

    CVF_ASSERT(wellPath);
    CVF_ASSERT(collection);

    RiaApplication* app = RiaApplication::instance();

    QString projectFolder = app->currentProjectPath();
    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallback("COMPLETIONS", projectFolder);

    RicCaseAndFileExportSettingsUi exportSettings;
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

    exportSettings.folder = defaultDir;

    caf::PdmUiPropertyViewDialog propertyDialog(Riu3DMainWindowTools::mainWindowWidget(), &exportSettings, "Export Fractures as Multi Segment Wells", "");
    RicExportFeatureImpl::configureForExport(&propertyDialog);

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("COMPLETIONS", QFileInfo(exportSettings.folder).absolutePath());
        std::vector<RimWellPathFracture*> fractures;
        if (fracture)
        {
            fractures.push_back(fracture);
        }
        else
        {
            for (RimWellPathFracture* fracture : collection->activeFractures())
            {
                fractures.push_back(fracture);
            }
        }

        exportWellSegments(wellPath, fractures, exportSettings);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFracturesWellSegmentsFeature::setupActionLook(QAction* actionToSetup)
{
    if (caf::SelectionManager::instance()->selectedItemOfType<RimWellPathFracture>())
    {
        actionToSetup->setText("Export Fracture as Multi Segment Well");
    }
    else
    {
        actionToSetup->setText("Export Fractures as Multi Segment Well");
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportFracturesWellSegmentsFeature::isCommandEnabled()
{
    if (RiaApplication::enableDevelopmentFeatures() && caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPathFractureCollection>())
    {
        return true;
    }

    return false;
}
