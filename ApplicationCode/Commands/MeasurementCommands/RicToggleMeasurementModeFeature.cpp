/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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
CAF_CMD_SOURCE_INIT(RicTogglePolyMeasurementModeFeature, "RicTogglePolyMeasurementModeFeature");

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
    if (meas->measurementMode() == RimMeasurement::MEASURE_REGULAR)
    {
        meas->setMeasurementMode(RimMeasurement::MEASURE_DISABLED);
    }
    else
    {
        meas->setMeasurementMode(RimMeasurement::MEASURE_REGULAR);
    }
    
    refreshActionLook();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleMeasurementModeFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Measurement Mode");
    actionToSetup->setIcon(QIcon(":/Ruler24x24.png"));
    actionToSetup->setCheckable(true);

    auto* meas = measurement();
    if (meas && meas->measurementMode() == RimMeasurement::MEASURE_REGULAR)
    {
        applyShortcutWithHintToAction(actionToSetup, QKeySequence(Qt::Key_Escape));
    }
    else
    {
        applyShortcutWithHintToAction(actionToSetup, QKeySequence());
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RicToggleMeasurementModeFeature::isCommandChecked()
{
    auto meas = measurement();
    if (meas)
    {
        return meas->measurementMode() == RimMeasurement::MEASURE_REGULAR;
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTogglePolyMeasurementModeFeature::onActionTriggered(bool isChecked)
{
    auto meas = measurement();
    if (meas->measurementMode() == RimMeasurement::MEASURE_POLYLINE)
    {
        meas->setMeasurementMode(RimMeasurement::MEASURE_DISABLED);
    }
    else
    {
        meas->setMeasurementMode(RimMeasurement::MEASURE_POLYLINE);
    }

    refreshActionLook();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTogglePolyMeasurementModeFeature::setupActionLook(QAction* actionToSetup)
{
    actionToSetup->setText("Poly Line Measurement Mode");
    actionToSetup->setIcon(QIcon(":/RulerPoly24x24.png"));
    actionToSetup->setCheckable(true);

    auto* meas = measurement();
    if (meas && meas->measurementMode() == RimMeasurement::MEASURE_POLYLINE)
    {
        applyShortcutWithHintToAction(actionToSetup, QKeySequence(Qt::Key_Escape));
    }
    else
    {
        applyShortcutWithHintToAction(actionToSetup, QKeySequence());
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTogglePolyMeasurementModeFeature::isCommandChecked()
{
    auto meas = measurement();
    if (meas)
    {
        return meas->measurementMode() == RimMeasurement::MEASURE_POLYLINE;
    }

    return false;
}
