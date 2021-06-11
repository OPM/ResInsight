/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020 Equinor ASA
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

#include "RicNewWellMeasurementCurveFeature.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RimTools.h"
#include "RimWellLogTrack.h"
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewWellMeasurementCurveFeature, "RicNewWellMeasurementCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellMeasurementCurveFeature::isCommandEnabled()
{
    if ( RicWellLogPlotCurveFeatureImpl::parentWellRftPlot() ) return false;

    return ( caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellLogTrack>() != nullptr );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellMeasurementCurveFeature::onActionTriggered( bool isChecked )
{
    RimWellLogTrack* wellLogPlotTrack = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellLogTrack>();
    if ( wellLogPlotTrack )
    {
        RimWellPath* wellPath = nullptr;

        RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();
        if ( wellPathCollection )
        {
            const RimWellMeasurementCollection* measurementCollection = wellPathCollection->measurementCollection();

            QString measurementKind = "Unknown";

            if ( measurementCollection && !measurementCollection->measurements().empty() )
            {
                RimWellMeasurement* firstMeasurement = measurementCollection->measurements().front();

                wellPath = wellPathCollection->tryFindMatchingWellPath( firstMeasurement->wellName() );

                measurementKind = firstMeasurement->kind();
            }

            RicWellLogTools::addWellMeasurementCurve( wellLogPlotTrack, wellPath, measurementKind );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellMeasurementCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
    actionToSetup->setText( "New Well Measurement Curve" );
}
