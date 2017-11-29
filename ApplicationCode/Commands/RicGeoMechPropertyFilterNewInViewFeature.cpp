/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicGeoMechPropertyFilterNewInViewFeature.h"

#include "RicGeoMechPropertyFilterNewExec.h"
#include "RicGeoMechPropertyFilterFeatureImpl.h"

#include "RiaApplication.h"

#include "RimGeoMechCellColors.h"
#include "RimGeoMechPropertyFilterCollection.h"
#include "RimGeoMechView.h"
#include "RimView.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicGeoMechPropertyFilterNewInViewFeature, "RicGeoMechPropertyFilterNewInViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicGeoMechPropertyFilterNewInViewFeature::isCommandEnabled()
{
    RimView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return false;
    
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(view);
    if (!geoMechView) return false;

    RimGeoMechCellColors* cellColors = geoMechView->cellResult().p();
    if (!cellColors) return false;

    RimGeoMechPropertyFilterCollection* filterCollection = geoMechView->geoMechPropertyFilterCollection();
    if (filterCollection)
    {
        return RicGeoMechPropertyFilterFeatureImpl::isPropertyFilterCommandAvailable(filterCollection);
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNewInViewFeature::onActionTriggered(bool isChecked)
{
    RimView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return;
    RimGeoMechView* eclView = dynamic_cast<RimGeoMechView*>(view);
    if (!eclView) return;

    RicGeoMechPropertyFilterNewExec* filterExec = new RicGeoMechPropertyFilterNewExec(eclView->geoMechPropertyFilterCollection());
    caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicGeoMechPropertyFilterNewInViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CellFilter_Values.png"));
    actionToSetup->setText("New Property Filter");
}
