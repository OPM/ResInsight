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

#include "RicNewRftSegmentWellLogCurveFeature.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RiaApplication.h"
#include "RiaRftDefines.h"

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

CAF_CMD_SOURCE_INIT( RicNewRftSegmentWellLogCurveFeature, "RicNewRftSegmentWellLogCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewRftSegmentWellLogCurveFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogCurveFeature::onActionTriggered( bool isChecked )
{
    auto rftCase = caf::SelectionManager::instance()->selectedItemOfType<RimRftCase>();
    if ( !rftCase ) return;

    RimSummaryCase* summaryCase = nullptr;
    rftCase->firstAncestorOfType( summaryCase );

    auto plot = RicNewWellLogPlotFeatureImpl::createHorizontalWellLogPlot();

    QString resultName = "SEGGRAT";

    {
        auto branchType = RiaDefines::RftBranchType::RFT_TUBING;

        appendTrackAndCurveForBranchType( plot, resultName, branchType, summaryCase );
    }
    {
        auto branchType = RiaDefines::RftBranchType::RFT_DEVICE;

        appendTrackAndCurveForBranchType( plot, resultName, branchType, summaryCase );
    }
    {
        auto branchType = RiaDefines::RftBranchType::RFT_ANNULUS;

        appendTrackAndCurveForBranchType( plot, resultName, branchType, summaryCase );
    }

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );
    RiuPlotMainWindowTools::refreshToolbars();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogCurveFeature::appendTrackAndCurveForBranchType( RimWellLogPlot*           plot,
                                                                            const QString&            resultName,
                                                                            RiaDefines::RftBranchType branchType,
                                                                            RimSummaryCase*           summaryCase )
{
    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addPlot( plotTrack );
    plotTrack->setDescription( QString( "Track %1" ).arg( plot->plotCount() ) );

    plot->loadDataAndUpdate();

    auto curve = RicWellLogTools::addSummaryRftSegmentCurve( plotTrack, resultName, branchType, summaryCase );
    curve->loadDataAndUpdate( true );

    curve->updateAllRequiredEditors();
    RiuPlotMainWindowTools::setExpanded( curve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Append RFT Segment Curve" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
