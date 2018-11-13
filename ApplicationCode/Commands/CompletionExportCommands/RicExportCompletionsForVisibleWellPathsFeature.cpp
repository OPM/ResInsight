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

#include "RicExportCompletionsForVisibleWellPathsFeature.h"

#include "RiaApplication.h"

#include "RicWellPathExportCompletionDataFeature.h"

#include "RiuPlotMainWindow.h"

#include "RimWellPath.h"
#include "RimProject.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicExportCompletionsForVisibleWellPathsFeature, "RicExportCompletionsForVisibleWellPathsFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicExportCompletionsForVisibleWellPathsFeature::isCommandEnabled()
{
    bool foundWellPathCollection = false;
    std::vector<caf::PdmObject*> selectedObjects;
    caf::SelectionManager::instance()->objectsByType(&selectedObjects);
    for (caf::PdmObject* object : selectedObjects)
    {
        RimWellPathCollection* wellPathCollection;
        
        object->firstAncestorOrThisOfType(wellPathCollection);
        if (wellPathCollection)
        {
            foundWellPathCollection = true;
            break;
        }
    }
    
    if (!foundWellPathCollection)
    {
        return false;
    }

    std::vector<RimWellPath*> wellPaths = visibleWellPaths();

    if (wellPaths.empty())
    {
        return false;
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForVisibleWellPathsFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellPath*> wellPaths = visibleWellPaths();
    CVF_ASSERT(wellPaths.size() > 0);

    std::vector<RimSimWellInView*> simWells;
    QString                        dialogTitle = "Export Completion Data for Visible Well Paths";

    RicWellPathExportCompletionDataFeature::prepareExportSettingsAndExportCompletions(dialogTitle, wellPaths, simWells);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicExportCompletionsForVisibleWellPathsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Export Completion Data for Visible Well Paths");
    actionToSetup->setIcon(QIcon(":/ExportCompletionsSymbol16x16.png"));

}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimWellPath*> RicExportCompletionsForVisibleWellPathsFeature::visibleWellPaths()
{
    std::vector<RimWellPath*> wellPaths;

    {
        std::vector<RimWellPathCollection*> wellPathCollections;
        caf::SelectionManager::instance()->objectsByType(&wellPathCollections);

        if (wellPathCollections.empty())
        {
            std::vector<RimWellPath*> selectedWellPaths;
            caf::SelectionManager::instance()->objectsByType(&selectedWellPaths);

            if (!selectedWellPaths.empty())
            {
                RimWellPathCollection* parent = nullptr;
                selectedWellPaths[0]->firstAncestorOrThisOfType(parent);
                if (parent)
                {
                    wellPathCollections.push_back(parent);
                }
            }
        }

        if (!wellPathCollections.empty())
        {
            for (auto wellPathCollection : wellPathCollections)
            {
                for (const auto& wellPath : wellPathCollection->wellPaths())
                {
                    if (wellPath->showWellPath())
                    {
                        wellPaths.push_back(wellPath);
                    }
                }
            }
        }
        else
        {
            // No well path or well path collection selected 

            auto allWellPaths = RiaApplication::instance()->project()->allWellPaths();
            for (const auto& w : allWellPaths)
            {
                if (w->showWellPath())
                {
                    wellPaths.push_back(w);
                }
            }

        }
    }

    std::set<RimWellPath*> uniqueWellPaths(wellPaths.begin(), wellPaths.end());
    wellPaths.assign(uniqueWellPaths.begin(), uniqueWellPaths.end());

    return wellPaths;
}
