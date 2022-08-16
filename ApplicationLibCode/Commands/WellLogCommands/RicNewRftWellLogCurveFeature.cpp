/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2022     Equinor ASA
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

#include "RicNewRftWellLogCurveFeature.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RiaApplication.h"

#include "RigWellLogCurveData.h"

#include "RimRftCase.h"
#include "RimSummaryCase.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogPlot.h"
#include "RimWellLogTrack.h"

#include "Riu3dSelectionManager.h"
#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

#include <vector>

CAF_CMD_SOURCE_INIT( RicNewRftWellLogCurveFeature, "RicNewRftWellLogCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewRftWellLogCurveFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftWellLogCurveFeature::onActionTriggered( bool isChecked )
{
    auto rftCase = caf::SelectionManager::instance()->selectedItemOfType<RimRftCase>();
    if ( !rftCase ) return;

    RimSummaryCase* summaryCase = nullptr;
    rftCase->firstAncestorOfType( summaryCase );

    auto plot = RicNewWellLogPlotFeatureImpl::createWellLogPlot();

    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addPlot( plotTrack );
    plotTrack->setDescription( QString( "Track %1" ).arg( plot->plotCount() ) );

    plot->loadDataAndUpdate();

    auto curve = RicWellLogTools::addSummaryRftCurve( plotTrack, summaryCase );
    curve->loadDataAndUpdate( true );

    curve->updateAllRequiredEditors();

    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftWellLogCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Append RFT Well Log Curve" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
