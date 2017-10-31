/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicWellLogFileCloseFeature.h"

#include "RiaApplication.h"

#include "RimWellPath.h"
#include "RimWellLogFile.h"
#include "RimWellLogPlot.h"
#include "RimWellPltPlot.h"
#include "RimWellRftPlot.h"
#include "RimWellAllocationPlot.h"
#include "RimViewWindow.h"

#include "cafSelectionManagerTools.h"
#include "cafPdmObjectHandle.h"
#include "cafPdmUiObjectEditorHandle.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicWellLogFileCloseFeature, "RicWellLogFileCloseFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicWellLogFileCloseFeature::isCommandEnabled()
{
    std::vector<RimWellLogFile*> objects = caf::selectedObjectsByType<RimWellLogFile*>();
    return objects.size() > 0;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogFileCloseFeature::onActionTriggered(bool isChecked)
{
    std::vector<RimWellLogFile*> objects = caf::selectedObjectsByType<RimWellLogFile*>();

    if (objects.size() == 0) return;

    for (const auto& wellLogFile : objects)
    {
        RimWellPath* parentWellPath;
        wellLogFile->firstAncestorOrThisOfType(parentWellPath);

        if (parentWellPath != nullptr)
        {
            std::set<RimViewWindow*> referringPlots = referringWellLogPlots(wellLogFile);
            parentWellPath->deleteWellLogFile(wellLogFile);

            for (RimViewWindow* plot : referringPlots)
            {
                plot->loadDataAndUpdate();
            }
        }
        parentWellPath->updateConnectedEditors();
    }

    caf::PdmUiObjectEditorHandle::updateUiAllObjectEditors();
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogFileCloseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Close Well Log File(s)");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
std::set<RimViewWindow*> RicWellLogFileCloseFeature::referringWellLogPlots(const RimWellLogFile* wellLogFile)
{
    // Remove all curves displaying data from the specified wellLogFile
    std::vector<caf::PdmObjectHandle*> referringObjects;
    wellLogFile->objectsWithReferringPtrFields(referringObjects);

    std::set<RimViewWindow*> plots;
    for (const auto& obj : referringObjects)
    {
        RimWellAllocationPlot* allocationPlot;
        RimWellPltPlot* pltPlot;
        RimWellRftPlot* rftPlot;
        RimWellLogPlot* wellLogPlot;

        obj->firstAncestorOrThisOfType(allocationPlot);
        obj->firstAncestorOrThisOfType(pltPlot);
        obj->firstAncestorOrThisOfType(rftPlot);
        obj->firstAncestorOrThisOfType(wellLogPlot);

        RimViewWindow* plot = 
            allocationPlot ?    dynamic_cast<RimViewWindow*>(allocationPlot) :
            pltPlot ?           dynamic_cast<RimViewWindow*>(pltPlot) :
            rftPlot ?           dynamic_cast<RimViewWindow*>(rftPlot) :
            wellLogPlot ?       dynamic_cast<RimViewWindow*>(wellLogPlot) :
                                nullptr;

        if (plot != nullptr)
        {
            plots.insert(plot);
        }
    }
    return plots;
}
