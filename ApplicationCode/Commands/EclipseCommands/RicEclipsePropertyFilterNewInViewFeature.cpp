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

#include "RicEclipsePropertyFilterNewInViewFeature.h"

#include "RicEclipsePropertyFilterNewExec.h"
#include "RicEclipsePropertyFilterFeatureImpl.h"

#include "RiaApplication.h"

#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "Rim3dView.h"

#include "cafCmdExecCommandManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicEclipsePropertyFilterNewInViewFeature, "RicEclipsePropertyFilterNewInViewFeature");

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicEclipsePropertyFilterNewInViewFeature::isCommandEnabled()
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return false;
    
    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
    if (!eclView) return false;

    RimEclipseCellColors* cellColors = eclView->cellResult().p();
    if (!cellColors) return false;

    if (RiaDefines::isPerCellFaceResult(cellColors->resultVariable())) return false;

    RimEclipsePropertyFilterCollection* filterCollection = eclView->eclipsePropertyFilterCollection();
    if (filterCollection)
    {
        return RicEclipsePropertyFilterFeatureImpl::isPropertyFilterCommandAvailable(filterCollection);
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewInViewFeature::onActionTriggered(bool isChecked)
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if (!view) return;
    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(view);
    if (!eclView) return;

    RicEclipsePropertyFilterNewExec* filterExec = new RicEclipsePropertyFilterNewExec(eclView->eclipsePropertyFilterCollection());
    caf::CmdExecCommandManager::instance()->processExecuteCommand(filterExec);
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicEclipsePropertyFilterNewInViewFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setIcon(QIcon(":/CellFilter_Values.png"));
    actionToSetup->setText("Property Filter");
}
