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

#include "RicToggleMeasurementModeFeature.h"

#include "RiaApplication.h"

#include "Rim3dView.h"
#include "RimMeasurement.h"
#include "RimProject.h"

#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT(RicToggleMeasurementModeFeature, "RicToggleMeasurementModeFeature");

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleMeasurementModeFeature::refreshActionLook()
{
    setupActionLook(action());
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleMeasurementModeFeature::isCommandEnabled()
{
    return activeView() != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleMeasurementModeFeature::onActionTriggered(bool isChecked)
{
    auto meas = measurement();
    meas->setMeasurementMode(!meas->isInMeasurementMode());
    
    refreshActionLook();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleMeasurementModeFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Measurement Mode");
    actionToSetup->setIcon(QIcon(":/Ruler16x16.png"));

/*
    auto* meas = measurement();
    if (meas && meas->isInMeasurementMode())
        actionToSetup->setIcon(QIcon(":/NoRuler16x16.png"));
    else
        actionToSetup->setIcon(QIcon(":/Ruler16x16.png"));
*/
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicToggleMeasurementModeFeature::isCommandChecked()
{
    auto meas = measurement();
    if (meas)
    {
        return meas->isInMeasurementMode();
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimMeasurement* RicToggleMeasurementModeFeature::measurement() const
{
    return RiaApplication::instance()->project()->measurement();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RicToggleMeasurementModeFeature::activeView() const
{
    auto view = RiaApplication::instance()->activeReservoirView();
    return view;
}
