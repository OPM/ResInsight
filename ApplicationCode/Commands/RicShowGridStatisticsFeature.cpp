/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include "RicShowGridStatisticsFeature.h"

#include "RiaApplication.h"
#include "RicGridStatisticsDialog.h"
#include "RicWellLogTools.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimEclipseView.h"
#include "RimGeoMechView.h"
#include "RimProject.h"

#include "cafSelectionManagerTools.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicShowGridStatisticsFeature, "RicShowGridStatisticsFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowGridStatisticsFeature::isCommandEnabled()
{
    if (RicWellLogTools::isWellPathOrSimWellSelectedInView()) return false;
    
    return RiaApplication::instance()->activeGridView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowGridStatisticsFeature::onActionTriggered(bool isChecked)
{
    RimGridView * activeView = RiaApplication::instance()->activeGridView();

    if (activeView)
    {
        activeView->overlayInfoConfig()->showStatisticsInfoDialog();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowGridStatisticsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Grid Statistics");
    actionToSetup->setIcon(QIcon(":/statistics.png"));  // Todo: Change icon
}
