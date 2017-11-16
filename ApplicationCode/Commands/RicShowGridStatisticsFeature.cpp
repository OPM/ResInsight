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

#include "RimProject.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimEclipseView.h"

#include "cafSelectionManagerTools.h"

#include <QAction>


CAF_CMD_SOURCE_INIT(RicShowGridStatisticsFeature, "RicShowGridStatisticsFeature");


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicShowGridStatisticsFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowGridStatisticsFeature::onActionTriggered(bool isChecked)
{
    auto eclipseView = caf::firstAncestorOfTypeFromSelectedObject<RimEclipseView*>();

    if (eclipseView && eclipseView->overlayInfoConfig())
    {
        eclipseView->overlayInfoConfig()->showStatisticsInfoDialog();
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
void RicShowGridStatisticsFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Grid statistics");
    actionToSetup->setIcon(QIcon(":/statistics.png"));  // Todo: Change icon
}
