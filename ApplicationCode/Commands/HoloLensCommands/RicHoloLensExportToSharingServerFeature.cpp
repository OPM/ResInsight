/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018     Statoil ASA
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

#include "RicHoloLensExportToSharingServerFeature.h"

#include "VdeFileExporter.h"

#include "RiaApplication.h"
#include "RimGridView.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicHoloLensExportToSharingServerFeature, "RicHoloLensExportToSharingServerFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicHoloLensExportToSharingServerFeature::isCommandEnabled()
{
    // Return true if a valid session is active

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportToSharingServerFeature::onActionTriggered(bool isChecked)
{
    RimGridView* activeView = RiaApplication::instance()->activeGridView();

    if (!activeView) return;

    VdeFileExporter exporter("dummypath");
    exporter.exportViewContents(*activeView);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHoloLensExportToSharingServerFeature::setupActionLook(QAction* actionToSetup)
{
    //actionToSetup->setIcon(QIcon(":/Save.png"));
    actionToSetup->setText("HoloLens : Export to Sharing Server");
}
