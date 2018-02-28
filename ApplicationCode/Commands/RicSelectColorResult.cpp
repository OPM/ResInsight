/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Statoil ASA
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

#include "RicSelectColorResult.h"

#include "RiaApplication.h"
#include "RicWellLogTools.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseView.h"
#include "RimGeoMechCellColors.h"
#include "RimGeoMechView.h"
#include "RimGridView.h"
#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicSelectColorResult, "RicSelectColorResult");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectColorResult::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Color Result");
    actionToSetup->setIcon(QIcon(":/CellResult.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSelectColorResult::isCommandEnabled()
{
    if (RicWellLogTools::isWellPathOrSimWellSelectedInView()) return false;

    return RiaApplication::instance()->activeGridView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectColorResult::onActionTriggered(bool isChecked)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(activeView);
    if (eclView)
    {
        Riu3DMainWindowTools::selectAsCurrentItem(eclView->cellResult());
        return;
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(activeView);
    if (geoMechView)
    {
        Riu3DMainWindowTools::selectAsCurrentItem(geoMechView->cellResult());
    }
}
