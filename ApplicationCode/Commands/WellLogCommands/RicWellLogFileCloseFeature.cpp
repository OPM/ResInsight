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

#include "RimWellPath.h"
#include "RimWellLogFile.h"

#include "cafSelectionManagerTools.h"

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
            parentWellPath->deleteWellLogFile(wellLogFile);
        }
        parentWellPath->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicWellLogFileCloseFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Close Well Log File(s)");
    actionToSetup->setIcon(QIcon(":/Erase.png"));
}
