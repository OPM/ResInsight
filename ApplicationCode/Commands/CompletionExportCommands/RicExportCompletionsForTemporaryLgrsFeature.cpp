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

#include "RicExportCompletionsForTemporaryLgrsFeature.h"

#include "RiaApplication.h"

#include "RicWellPathExportCompletionDataFeature.h"

#include "RigMainGrid.h"

#include "RiuPlotMainWindow.h"

#include "RimEclipseCase.h"
#include "RimGridCollection.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManagerTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicExportCompletionsForTemporaryLgrsFeature, "RicExportCompletionsForTemporaryLgrsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionsForTemporaryLgrsFeature::isCommandEnabled()
{
    if (wellPathsAssociatedWithLgrs().empty())
    {
        return false;
    }

    std::vector<RimGridInfoCollection*> selGridInfos = caf::selectedObjectsByTypeStrict<RimGridInfoCollection*>();
    return selGridInfos.size() == 1 && selGridInfos.front()->uiName() == RimGridCollection::temporaryGridUiName();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForTemporaryLgrsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = wellPathsAssociatedWithLgrs();
    if (wellPaths.empty())
    {
        return;
    }

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "Export Completion Data for Temporary LGRs";

    RicWellPathExportCompletionDataFeature::prepareExportSettingsAndExportCompletions(dialogTitle, wellPaths, simWells);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForTemporaryLgrsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Completion Data");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicExportCompletionsForTemporaryLgrsFeature::wellPathsAssociatedWithLgrs()
{
    std::vector<RimWellPath*> wellPaths;

    auto selectedEclipseCase = caf::firstAncestorOfTypeFromSelectedObject<RimEclipseCase*>();
    if (selectedEclipseCase)
    {
        auto mainGrid = selectedEclipseCase->mainGrid();

        std::set<QString> wellPathNames;

        for (size_t i = 0; i < mainGrid->gridCount(); i++)
        {
            const RigGridBase* grid = mainGrid->gridByIndex(i);

            if (!grid->associatedWellPathName().empty())
            {
                wellPathNames.insert(QString::fromStdString(grid->associatedWellPathName()));
            }
        }

        auto project = RiaApplication::instance()->project();
        for (const auto& wellPathName : wellPathNames)
        {
            auto wellPath = project->wellPathByName(wellPathName);
            if (wellPath)
            {
                wellPaths.push_back(wellPath);
            }
        }
    }

    return wellPaths;
}
