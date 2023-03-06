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

#include "RicNewMultiPhaseRftSegmentPlotFeature.h"
#include "RicNewRftSegmentWellLogPlotFeature.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogPlotCurveFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RiaApplication.h"
#include "RiaLogging.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaRftDefines.h"

#include "RifReaderOpmRft.h"

#include "RimRftCase.h"
#include "RimRftTopologyCurve.h"
#include "RimSummaryCase.h"
#include "RimWellLogPlot.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewMultiPhaseRftSegmentPlotFeature, "RicNewMultiPhaseRftSegmentPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewMultiPhaseRftSegmentPlotFeature::isCommandEnabled()
{
    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPhaseRftSegmentPlotFeature::onActionTriggered( bool isChecked )
{
    auto rftCase = caf::SelectionManager::instance()->selectedItemOfType<RimRftCase>();
    if ( !rftCase ) return;

    RimSummaryCase* summaryCase = nullptr;
    rftCase->firstAncestorOfType( summaryCase );
    if ( !summaryCase ) return;

    auto rftReader = summaryCase->rftReader();
    if ( !rftReader )
    {
        RiaLogging::error( "Could not open RFT file or no RFT file present. " );
        return;
    }

    auto plot = RicNewWellLogPlotFeatureImpl::createHorizontalWellLogPlot();

    QString wellName = "Unknown";

    auto wellNames = rftReader->wellNames();
    if ( !wellNames.empty() ) wellName = *wellNames.begin();

    appendTrackAndCurveForBranchType( plot,
                                      "Reservoir Rates",
                                      { "CONGRAT", "CONORAT", "CONWRAT" },
                                      wellName,
                                      RiaDefines::RftBranchType::RFT_ANNULUS,
                                      summaryCase );

    {
        for ( auto branchType :
              { RiaDefines::RftBranchType::RFT_ANNULUS, RiaDefines::RftBranchType::RFT_DEVICE, RiaDefines::RftBranchType::RFT_TUBING } )
        {
            QString trackName = caf::AppEnum<RiaDefines::RftBranchType>::uiText( branchType );
            trackName += " Rates";

            appendTrackAndCurveForBranchType( plot, trackName, { "SEGGRAT", "SEGORAT", "SEGWRAT" }, wellName, branchType, summaryCase );
        }
    }

    RicNewRftSegmentWellLogPlotFeature::appendPressureTrack( plot, wellName, summaryCase );
    RicNewRftSegmentWellLogPlotFeature::appendConnectionFactorTrack( plot, wellName, summaryCase );
    RicNewRftSegmentWellLogPlotFeature::appendTopologyTrack( plot, wellName, summaryCase );

    RicNewRftSegmentWellLogPlotFeature::updateUnitTexts( plot );
    plot->loadDataAndUpdate();
    plot->updateTrackVisibility();

    RiaPlotWindowRedrawScheduler::instance()->performScheduledUpdatesAndReplots();
    plot->updateLayout();

    RiuPlotMainWindowTools::onObjectAppended( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPhaseRftSegmentPlotFeature::appendTrackAndCurveForBranchType( RimWellLogPlot*             plot,
                                                                              const QString&              trackName,
                                                                              const std::vector<QString>& resultNames,
                                                                              const QString&              wellName,
                                                                              RiaDefines::RftBranchType   branchType,
                                                                              RimSummaryCase*             summaryCase )
{
    auto plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogTrackWithAutoUpdate();
    plot->addPlot( plotTrack );
    plotTrack->setDescription( trackName );

    for ( const auto& resultName : resultNames )
    {
        auto curve = RicWellLogTools::addSummaryRftSegmentCurve( plotTrack, resultName, wellName, branchType, summaryCase );
        curve->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );

        QString templateText = RiaDefines::namingVariableResultName() + ", " + RiaDefines::namingVariableResultType();
        curve->setCurveNameTemplateText( templateText );

        if ( resultName == "SEGGRAT" || resultName == "CONGRAT" )
        {
            curve->setScaleFactor( 1e-3 );
        }
        curve->setFillStyle( Qt::SolidPattern );

        curve->setIsStacked( true );

        curve->updateAllRequiredEditors();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPhaseRftSegmentPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create RFT Multi Phase Segment Plot" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
