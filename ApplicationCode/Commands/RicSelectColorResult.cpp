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
#include "Rim2dIntersectionView.h"
#include "RimIntersection.h"

#include "Riu3DMainWindowTools.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicSelectColorResult, "RicSelectColorResult");

//--------------------------------------------------------------------------------------------------
/// Internal function
//--------------------------------------------------------------------------------------------------
RimGridView* gridViewFrom2dIntersectionView(const Rim2dIntersectionView* int2dView)
{
    RimGridView* gridView = nullptr;
    int2dView->intersection()->firstAncestorOrThisOfType(gridView);
    return gridView;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectColorResult::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Select Color Result");
    actionToSetup->setIcon(QIcon(":/CellResult.png"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicSelectColorResult::isCommandEnabled()
{
    if (RicWellLogTools::isWellPathOrSimWellSelectedInView()) return false;

    return RiaApplication::instance()->activeReservoirView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicSelectColorResult::onActionTriggered(bool isChecked)
{
    Rim3dView*              activeView = RiaApplication::instance()->activeReservoirView();
    Rim2dIntersectionView*  int2dView = dynamic_cast<Rim2dIntersectionView*>(activeView);
    RimGridView*            gridView = nullptr;

    if (int2dView)  gridView = gridViewFrom2dIntersectionView(int2dView);
    else            gridView = dynamic_cast<RimGridView*>(activeView);

    RimEclipseView* eclView = dynamic_cast<RimEclipseView*>(gridView);
    if (eclView)
    {
        Riu3DMainWindowTools::selectAsCurrentItem(eclView->cellResult(), int2dView == nullptr);
        return;
    }

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>(gridView);
    if (geoMechView)
    {
        Riu3DMainWindowTools::selectAsCurrentItem(geoMechView->cellResult(), int2dView== nullptr);
    }
}
