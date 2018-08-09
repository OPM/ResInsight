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

#include "RicExportFishbonesWellSegmentsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RicExportFeatureImpl.h"

#include "RimProject.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimWellPath.h"
#include "RimEclipseCase.h"

#include "RigMainGrid.h"
#include "RigEclipseCaseData.h"
#include "RigWellPath.h"

#include "Riu3DMainWindowTools.h"

#include "cafSelectionManager.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafUtils.h"

#include "cvfMath.h"

#include <QAction>
#include <QDir>


CAF_CMD_SOURCE_INIT(RicExportFishbonesWellSegmentsFeature, "RicExportFishbonesWellSegmentsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::onActionTriggered(bool isChecked)
{
    RimFishbonesCollection* fishbonesCollection = selectedFishbonesCollection();
    RimWellPath* wellPath = selectedWellPath();
    CVF_ASSERT(fishbonesCollection);
    CVF_ASSERT(wellPath);

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

    caf::PdmUiPropertyViewDialog propertyDialog(Riu3DMainWindowTools::mainWindowWidget(), &exportSettings, "Export Well Segments", "");
    RicExportFeatureImpl::configureForExport(&propertyDialog);

    if (propertyDialog.exec() == QDialog::Accepted)
    {
        RiaApplication::instance()->setLastUsedDialogDirectory("COMPLETIONS", QFileInfo(exportSettings.folder).absolutePath());

        std::vector<RimFishbonesMultipleSubs*> fishbonesSubs;
        for (RimFishbonesMultipleSubs* subs : fishbonesCollection->fishbonesSubs)
        {
            fishbonesSubs.push_back(subs);
        }

        exportWellSegments(wellPath, fishbonesSubs, exportSettings);
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimFishbonesCollection* RicExportFishbonesWellSegmentsFeature::selectedFishbonesCollection()
{
    RimFishbonesCollection* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimWellPath* RicExportFishbonesWellSegmentsFeature::selectedWellPath()
{
    RimWellPath* objToFind = nullptr;
    
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (objHandle)
    {
        objHandle->firstAncestorOrThisOfType(objToFind);
    }

    return objToFind;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Well Segments");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportFishbonesWellSegmentsFeature::isCommandEnabled()
{
    if (selectedFishbonesCollection())
    {
        return true;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportFishbonesWellSegmentsFeature::exportWellSegments(const RimWellPath* wellPath, const std::vector<RimFishbonesMultipleSubs*>& fishbonesSubs, const RicCaseAndFileExportSettingsUi& settings)
{
    if (settings.caseToApply() == nullptr)
    {
        RiaLogging::error("Export Well Segments: Cannot export completions data without specified eclipse case");
        return;
    }

    QString fileName = QString("%1-Welsegs").arg(settings.caseToApply()->caseUserDescription());
    fileName = caf::Utils::makeValidFileBasename(fileName);

    QDir exportFolder(settings.folder());
    if (!exportFolder.exists())
    {
        bool createdPath = exportFolder.mkpath(".");
        if (createdPath)
            RiaLogging::info("Created export folder " + settings.folder());
        else
            RiaLogging::error("Selected output folder does not exist, and could not be created.");
    }

    QString filePath = exportFolder.filePath(fileName);
    QFile   exportFile(filePath);
    if (!exportFile.open(QIODevice::WriteOnly))
    {
        RiaLogging::error(QString("Export Well Segments: Could not open the file: %1").arg(filePath));
        return;
    }
    
    RicMultiSegmentWellExportInfo exportInfo = RicWellPathExportCompletionDataFeatureImpl::generateFishbonesMSWExportInfo(settings.caseToApply, wellPath, fishbonesSubs);

    QTextStream stream(&exportFile);
    RifEclipseDataTableFormatter formatter(stream);
    RicWellPathExportCompletionDataFeatureImpl::generateWelsegsTable(formatter,  exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateCompsegsTable(formatter, exportInfo);
    RicWellPathExportCompletionDataFeatureImpl::generateWsegvalvTable(formatter, exportInfo);
}
