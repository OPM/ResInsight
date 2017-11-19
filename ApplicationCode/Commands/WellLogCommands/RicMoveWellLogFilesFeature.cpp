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

#include "RicMoveWellLogFilesFeature.h"

#include "RiaApplication.h"
#include "RimProject.h"
#include "RimWellPath.h"
#include "RimWellLogFile.h"
#include "RiuMainWindow.h"

#include "cafPdmUiObjectEditorHandle.h"
#include "cafSelectionManagerTools.h"

#include <QAction>
#include <QFileDialog>

CAF_CMD_SOURCE_INIT(RicMoveWellLogFilesFeature, "RicMoveWellLogFilesFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicMoveWellLogFilesFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicMoveWellLogFilesFeature::onActionTriggered(bool isChecked)
{
    const QVariant userData = this->userData();
    if (!userData.isNull() && userData.type() == QVariant::String)
    {
        RimProject* proj = RiaApplication::instance()->project();
        
        RimWellPath* destWellPath = proj->wellPathByName(userData.toString());
        RimWellLogFile* wellLogFile = caf::firstAncestorOfTypeFromSelectedObject<RimWellLogFile*>();
        RimWellPath* sourceWellPath = caf::firstAncestorOfTypeFromSelectedObject<RimWellPath*>();

        if (!destWellPath || !wellLogFile || !sourceWellPath) return;

        sourceWellPath->detachWellLogFile(wellLogFile);
        destWellPath->addWellLogFile(wellLogFile);

        sourceWellPath->updateConnectedEditors();
        destWellPath->updateConnectedEditors();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicMoveWellLogFilesFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Move Well Log File(s)");
    actionToSetup->setIcon(QIcon(":/Well.png"));
}
