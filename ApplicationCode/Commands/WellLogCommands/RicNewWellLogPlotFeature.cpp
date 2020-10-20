/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2015-     Statoil ASA
//  Copyright (C) 2015-     Ceetron Solutions AS
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

#include "RicNewWellLogPlotFeature.h"

#include "RicNewWellLogCurveExtractionFeature.h"
#include "RicNewWellLogFileCurveFeature.h"
#include "RicNewWellLogPlotFeatureImpl.h"

#include "RimProject.h"
#include "RimWellLogCurve.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "RicWellLogTools.h"

#include <QAction>

#include "cvfAssert.h"

CAF_CMD_SOURCE_INIT( RicNewWellLogPlotFeature, "RicNewWellLogPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellLogPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotFeature::onActionTriggered( bool isChecked )
{
    RimWellLogTrack*           plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack();
    RimWellLogExtractionCurve* curve =
        RicWellLogTools::addWellLogExtractionCurve( plotTrack, nullptr, nullptr, nullptr, nullptr, -1, true );
    curve->loadDataAndUpdate( true );
    RimWellLogPlot* plot = nullptr;
    plotTrack->firstAncestorOrThisOfTypeAsserted( plot );
    plot->zoomAll();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Well Log Plot" );
    actionToSetup->setIcon( QIcon( ":/WellLogPlot16x16.png" ) );
}
