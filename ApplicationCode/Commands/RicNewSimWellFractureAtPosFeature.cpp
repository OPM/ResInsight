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

#include "RicNewSimWellFractureAtPosFeature.h"

#include "RiaApplication.h"

#include "RimCase.h"
#include "RimEclipseWell.h"
#include "RimFracture.h"
#include "RimFractureCollection.h"
#include "RimProject.h"
 
#include "cafSelectionManager.h"

#include "cvfAssert.h"

#include "QAction.h"


CAF_CMD_SOURCE_INIT(RicNewSimWellFractureAtPosFeature, "RicNewSimWellFractureAtPosFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureAtPosFeature::onActionTriggered(bool isChecked)
{
 // Not yet implemented 
 // Infrastructure is missing for being able to obtain i j and k when right-clicking
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicNewSimWellFractureAtPosFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/CrossSection16x16.png"));
    actionToSetup->setText("New Fracture");
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicNewSimWellFractureAtPosFeature::isCommandEnabled()
{
    caf::PdmUiItem* pdmUiItem = caf::SelectionManager::instance()->selectedItem();
    if (!pdmUiItem) return false;

    caf::PdmObjectHandle* objHandle = dynamic_cast<caf::PdmObjectHandle*>(pdmUiItem);
    if (!objHandle) return false;

    RimEclipseWell* eclipseWell = nullptr;
    objHandle->firstAncestorOrThisOfType(eclipseWell);

    if (eclipseWell)
    {
        return true;
    }

    return false;
}
