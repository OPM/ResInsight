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

#include "RicNewRftSegmentWellLogPlotFeature.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RiaApplication.h"
#include "RiaRftDefines.h"

#include "RifReaderEclipseRft.h"
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

#include "RifReaderOpmRft.h"
#include "RimRftWellCompletionTrack.h"
#include <vector>

CAF_CMD_SOURCE_INIT( RicNewRftSegmentWellLogPlotFeature, "RicNewRftSegmentWellLogPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewRftSegmentWellLogPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::onActionTriggered( bool isChecked )
{
    auto rftCase = caf::SelectionManager::instance()->selectedItemOfType<RimRftCase>();
    if ( !rftCase ) return;

    RimSummaryCase* summaryCase = nullptr;
    rftCase->firstAncestorOfType( summaryCase );
    if ( !summaryCase ) return;

    auto plot = RicNewWellLogPlotFeatureImpl::createHorizontalWellLogPlot();

    QString wellName = "Unknown";

    auto rftReader = summaryCase->rftReader();
    if ( rftReader )
    {
        auto wellNames = rftReader->wellNames();
        if ( !wellNames.empty() ) wellName = *wellNames.begin();
    }

    QString resultName = "SEGGRAT";

    {
        auto branchType = RiaDefines::RftBranchType::RFT_TUBING;

        appendTrackAndCurveForBranchType( plot, resultName, wellName, branchType, summaryCase );
    }
    {
        auto branchType = RiaDefines::RftBranchType::RFT_DEVICE;

        appendTrackAndCurveForBranchType( plot, resultName, wellName, branchType, summaryCase );
    }
    {
        auto branchType = RiaDefines::RftBranchType::RFT_ANNULUS;

        appendTrackAndCurveForBranchType( plot, resultName, wellName, branchType, summaryCase );
    }

    appendWellCompletionTrack( plot, wellName, summaryCase );

    RiuPlotMainWindowTools::onObjectAppended( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::appendTrackAndCurveForBranchType( RimWellLogPlot*           plot,
                                                                           const QString&            resultName,
                                                                           const QString&            wellName,
                                                                           RiaDefines::RftBranchType branchType,
                                                                           RimSummaryCase*           summaryCase )
{
    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addPlot( plotTrack );
    plotTrack->setDescription( QString( "Track %1" ).arg( plot->plotCount() ) );

    plot->loadDataAndUpdate();

    auto curve = RicWellLogTools::addSummaryRftSegmentCurve( plotTrack, resultName, wellName, branchType, summaryCase );
    curve->loadDataAndUpdate( true );

    curve->updateAllRequiredEditors();
    RiuPlotMainWindowTools::setExpanded( curve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::appendWellCompletionTrack( RimWellLogPlot* plot,
                                                                    const QString&  wellName,
                                                                    RimSummaryCase* summaryCase )
{
    // Add well log track
    auto track = new RimRftWellCompletionTrack();
    plot->addPlot( track );

    track->setDescription( "Well Completions" );
    track->setLegendsVisible( true );
    track->setShowWellPathAttributes( true );

    plot->loadDataAndUpdate();
    plot->zoomAll();

    auto rftReader = dynamic_cast<RifReaderOpmRft*>( summaryCase->rftReader() );
    track->configureForWellPath( wellName, rftReader );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create RFT Segment Plot" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
