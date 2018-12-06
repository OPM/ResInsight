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

#include "RicExportCompletionsWellSegmentsFeature.h"

#include "RiaApplication.h"
#include "RiaLogging.h"

#include "RifEclipseDataTableFormatter.h"

#include "RicExportFeatureImpl.h"
#include "RicMswExportInfo.h"
#include "RicWellPathExportCompletionDataFeatureImpl.h"
#include "RicWellPathExportMswCompletionsImpl.h"
#include "RicWellPathExportCompletionsFileTools.h"

#include "RimProject.h"
#include "RimFishboneWellPathCollection.h"
#include "RimFishbonesCollection.h"
#include "RimFishbonesMultipleSubs.h"
#include "RimPerforationCollection.h"
#include "RimWellPath.h"
#include "RimWellPathFractureCollection.h"
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


CAF_CMD_SOURCE_INIT(RicExportCompletionsWellSegmentsFeature, "RicExportCompletionsWellSegmentsFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsWellSegmentsFeature::onActionTriggered(bool isChecked)
{
    RimWellPath* wellPath = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
    CVF_ASSERT(wellPath);

    RimFishbonesCollection* fishbonesCollection =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimFishbonesCollection>();
    RimWellPathFractureCollection* fractureCollection =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPathFractureCollection>();
    RimPerforationCollection* perforationCollection =
        caf::SelectionManager::instance()->selectedItemAncestorOfType<RimPerforationCollection>();

    CVF_ASSERT(fishbonesCollection || fractureCollection || perforationCollection);

    RiaApplication* app = RiaApplication::instance();

    QString defaultDir = RiaApplication::instance()->lastUsedDialogDirectoryWithFallbackToProjectFolder("COMPLETIONS");

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
        RicExportCompletionDataSettingsUi completionExportSettings;
        completionExportSettings.caseToApply.setValue(exportSettings.caseToApply);
        completionExportSettings.folder.setValue(exportSettings.folder);

        completionExportSettings.includeFishbones = fishbonesCollection != nullptr && !fishbonesCollection->activeFishbonesSubs().empty();
        completionExportSettings.includeFractures = fractureCollection != nullptr && !fractureCollection->activeFractures().empty();
        completionExportSettings.includePerforations = perforationCollection != nullptr && !perforationCollection->activePerforations().empty();

        RicWellPathExportMswCompletionsImpl::exportWellSegmentsForAllCompletions(completionExportSettings, { wellPath });
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsWellSegmentsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Well Segments");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionsWellSegmentsFeature::isCommandEnabled()
{
    if (caf::SelectionManager::instance()->selectedItemAncestorOfType<RimFishbonesCollection>())
    {
        return true;
    }
    else if (caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPathFractureCollection>())
    {
        return true;
    }
    else if (caf::SelectionManager::instance()->selectedItemAncestorOfType<RimPerforationCollection>())
    {
        return true;
    }

    return false;
}
