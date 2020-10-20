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

#include "RiaGuiApplication.h"

#include "Rim3dView.h"
#include "RimGridView.h"
#include "RimMeasurement.h"
#include "RimProject.h"

#include "cafCmdFeatureManager.h"
#include "cafPdmUiPropertyViewDialog.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicToggleMeasurementModeFeature, "RicToggleMeasurementModeFeature" );
CAF_CMD_SOURCE_INIT( RicTogglePolyMeasurementModeFeature, "RicTogglePolyMeasurementModeFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleMeasurementModeFeature::refreshActionLook()
{
    setupActionLook( action() );
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
void RicToggleMeasurementModeFeature::onActionTriggered( bool isChecked )
{
    auto meas = measurement();
    if ( meas->measurementMode() == RimMeasurement::MEASURE_REGULAR )
    {
        meas->setMeasurementMode( RimMeasurement::MEASURE_DISABLED );
    }
    else
    {
        meas->setMeasurementMode( RimMeasurement::MEASURE_REGULAR );
    }

    refreshActionLook();
    refreshPolyMeasuremeantActionLook();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleMeasurementModeFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Measurement" );
    actionToSetup->setToolTip( "Single Line Measurement (Ctrl+M)" );
    actionToSetup->setIcon( QIcon( ":/Ruler.svg" ) );
    actionToSetup->setCheckable( true );

    auto* meas = measurement();
    if ( meas && meas->measurementMode() == RimMeasurement::MEASURE_REGULAR )
    {
        applyShortcutWithHintToAction( actionToSetup, QKeySequence( Qt::Key_Escape ) );
        applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+M" ) ) );
    }
    else
    {
        applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+M" ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicToggleMeasurementModeFeature::isCommandChecked()
{
    auto meas = measurement();
    if ( meas )
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
    RiaGuiApplication* app = RiaGuiApplication::instance();
    CAF_ASSERT( app && app->project() );

    return app->project()->measurement();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Rim3dView* RicToggleMeasurementModeFeature::activeView() const
{
    RiaApplication* app = RiaApplication::instance();
    CAF_ASSERT( app );
    auto view = dynamic_cast<Rim3dView*>( app->activeMainOrComparisonGridView() );
    return view;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicToggleMeasurementModeFeature::refreshPolyMeasuremeantActionLook()
{
    auto cmdFeature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicTogglePolyMeasurementModeFeature" );
    static_cast<RicTogglePolyMeasurementModeFeature*>( cmdFeature )->refreshActionLook();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTogglePolyMeasurementModeFeature::onActionTriggered( bool isChecked )
{
    auto meas = measurement();
    if ( meas->measurementMode() == RimMeasurement::MEASURE_POLYLINE )
    {
        meas->setMeasurementMode( RimMeasurement::MEASURE_DISABLED );
    }
    else
    {
        meas->setMeasurementMode( RimMeasurement::MEASURE_POLYLINE );
    }

    refreshActionLook();
    refreshMeasurementActionLook();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTogglePolyMeasurementModeFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Poly Line Measurement" );
    actionToSetup->setToolTip( "Poly Line Measurement (Ctrl+Shift+M)" );
    actionToSetup->setIcon( QIcon( ":/RulerPoly.svg" ) );
    actionToSetup->setCheckable( true );

    auto* meas = measurement();
    if ( meas && meas->measurementMode() == RimMeasurement::MEASURE_POLYLINE )
    {
        applyShortcutWithHintToAction( actionToSetup, QKeySequence( Qt::Key_Escape ) );
        applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+Shift+M" ) ) );
    }
    else
    {
        applyShortcutWithHintToAction( actionToSetup, QKeySequence( tr( "Ctrl+Shift+M" ) ) );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicTogglePolyMeasurementModeFeature::isCommandChecked()
{
    auto meas = measurement();
    if ( meas )
    {
        return meas->measurementMode() == RimMeasurement::MEASURE_POLYLINE;
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicTogglePolyMeasurementModeFeature::refreshMeasurementActionLook()
{
    auto cmdFeature = caf::CmdFeatureManager::instance()->getCommandFeature( "RicToggleMeasurementModeFeature" );
    static_cast<RicToggleMeasurementModeFeature*>( cmdFeature )->refreshActionLook();
}
