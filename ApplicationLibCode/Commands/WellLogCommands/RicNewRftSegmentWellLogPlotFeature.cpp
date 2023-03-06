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
#include "RiaColorTools.h"
#include "RiaLogging.h"
#include "RiaPlotWindowRedrawScheduler.h"
#include "RiaRftDefines.h"

#include "RifReaderOpmRft.h"

#include "RimRftCase.h"
#include "RimRftTopologyCurve.h"
#include "RimSummaryCase.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafSelectionManager.h"

#include <QAction>

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

    auto rftReader = summaryCase->rftReader();
    if ( !rftReader )
    {
        RiaLogging::error( "Could not open RFT file or no RFT file present. " );
        return;
    }

    auto plot = RicNewWellLogPlotFeatureImpl::createHorizontalWellLogPlot();

    QString wellName  = "Unknown";
    auto    wellNames = rftReader->wellNames();
    if ( !wellNames.empty() ) wellName = *wellNames.begin();

    {
        RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogTrackWithAutoUpdate();
        plot->addPlot( plotTrack );
        plotTrack->setDescription( "Reservoir Rates" );

        auto curve = createAndAddCurve( plotTrack, "CONGRAT", wellName, RiaDefines::RftBranchType::RFT_ANNULUS, summaryCase );
        curve->setScaleFactor( 1e-3 );
        curve->setFillStyle( Qt::SolidPattern );
    }

    for ( auto branchType :
          { RiaDefines::RftBranchType::RFT_ANNULUS, RiaDefines::RftBranchType::RFT_DEVICE, RiaDefines::RftBranchType::RFT_TUBING } )
    {
        QString resultName = "SEGGRAT";
        QString trackName  = caf::AppEnum<RiaDefines::RftBranchType>::uiText( branchType );
        trackName += " Rates";
        auto curve = appendTrackAndCurveForBranchType( plot, trackName, resultName, wellName, branchType, summaryCase );
        curve->setScaleFactor( 1e-3 );
        curve->setFillStyle( Qt::SolidPattern );
    }

    appendPressureTrack( plot, wellName, summaryCase );
    appendConnectionFactorTrack( plot, wellName, summaryCase );
    appendTopologyTrack( plot, wellName, summaryCase );

    plot->loadDataAndUpdate();
    plot->updateTrackVisibility();

    RiaPlotWindowRedrawScheduler::instance()->performScheduledUpdatesAndReplots();
    plot->updateLayout();

    RiuPlotMainWindowTools::onObjectAppended( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve* RicNewRftSegmentWellLogPlotFeature::appendTrackAndCurveForBranchType( RimWellLogPlot*           plot,
                                                                                          const QString&            trackName,
                                                                                          const QString&            resultName,
                                                                                          const QString&            wellName,
                                                                                          RiaDefines::RftBranchType branchType,
                                                                                          RimSummaryCase*           summaryCase )
{
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogTrackWithAutoUpdate();
    plot->addPlot( plotTrack );
    plotTrack->setDescription( trackName );

    auto curve = createAndAddCurve( plotTrack, resultName, wellName, branchType, summaryCase );

    curve->updateAllRequiredEditors();

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve* RicNewRftSegmentWellLogPlotFeature::createAndAddCurve( RimWellLogTrack*          track,
                                                                           const QString&            resultName,
                                                                           const QString&            wellName,
                                                                           RiaDefines::RftBranchType branchType,
                                                                           RimSummaryCase*           summaryCase )
{
    auto curve = RicWellLogTools::addSummaryRftSegmentCurve( track, resultName, wellName, branchType, summaryCase );
    curve->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );

    QString templateText = RiaDefines::namingVariableResultName() + ", " + RiaDefines::namingVariableResultType();
    curve->setCurveNameTemplateText( templateText );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RicNewRftSegmentWellLogPlotFeature::appendAdditionalDataSourceTrack( RimWellLogPlot* plot,
                                                                                                const QString&  wellName,
                                                                                                RimSummaryCase* summaryCase )
{
    auto additionalDataSourceTrack = new RimWellLogTrack();
    additionalDataSourceTrack->setDescription( "Additional Data Source Curves" );

    plot->addPlot( additionalDataSourceTrack );
    additionalDataSourceTrack->setShowWindow( false );

    QString resultName   = RiaDefines::segmentNumberResultName();
    QString templateText = RiaDefines::namingVariableResultName() + ", " + RiaDefines::namingVariableResultType();

    std::vector<RimPlotCurve*> curves;

    for ( auto branchType :
          { RiaDefines::RftBranchType::RFT_TUBING, RiaDefines::RftBranchType::RFT_DEVICE, RiaDefines::RftBranchType::RFT_ANNULUS } )
    {
        auto curve = RicWellLogTools::addSummaryRftSegmentCurve( additionalDataSourceTrack, resultName, wellName, branchType, summaryCase );
        curve->setFillStyle( Qt::NoBrush );
        curve->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );
        curve->setCurveNameTemplateText( templateText );

        curves.push_back( curve );
    }

    additionalDataSourceTrack->updateAllRequiredEditors();

    return curves;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::appendTopologyTrack( RimWellLogPlot* plot, const QString& wellName, RimSummaryCase* summaryCase )
{
    auto rftReader = dynamic_cast<RifReaderOpmRft*>( summaryCase->rftReader() );
    if ( !rftReader ) return;

    QDateTime dateTime;
    int       branchIndex = 1;

    auto timeSteps = rftReader->availableTimeSteps( wellName );
    if ( !timeSteps.empty() ) dateTime = *timeSteps.rbegin();

    auto track = new RimWellLogTrack();
    track->setDescription( "Topology" );
    track->enablePropertyAxis( false );
    track->setRowSpan( RimPlot::TWO );

    plot->addPlot( track );

    std::vector<RimPlotCurve*> additionalDataSourceCurves = appendAdditionalDataSourceTrack( plot, wellName, summaryCase );

    for ( auto branchType :
          { RiaDefines::RftBranchType::RFT_TUBING, RiaDefines::RftBranchType::RFT_DEVICE, RiaDefines::RftBranchType::RFT_ANNULUS } )
    {
        auto curve = RimRftTopologyCurve::createTopologyCurve( summaryCase, dateTime, wellName, branchIndex, branchType );
        curve->setAdditionalDataSources( additionalDataSourceCurves );
        curve->applyDefaultAppearance();
        track->addCurve( curve );
    }

    auto packerCurve = RimRftTopologyCurve::createPackerCurve( summaryCase, dateTime, wellName, branchIndex );
    packerCurve->setAdditionalDataSources( additionalDataSourceCurves );
    packerCurve->applyDefaultAppearance();
    track->addCurve( packerCurve );

    track->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::appendPressureTrack( RimWellLogPlot* plot, const QString& wellName, RimSummaryCase* summaryCase )
{
    auto track = RicNewWellLogPlotFeatureImpl::createWellLogTrackWithAutoUpdate();
    track->setAutoCheckStateBasedOnCurveData( true );
    track->setDescription( "Pressure" );

    plot->addPlot( track );

    QString resultName = "SEGPRES";
    for ( auto branchType :
          { RiaDefines::RftBranchType::RFT_TUBING, RiaDefines::RftBranchType::RFT_DEVICE, RiaDefines::RftBranchType::RFT_ANNULUS } )
    {
        auto curve = createAndAddCurve( track, resultName, wellName, branchType, summaryCase );
        auto color = RimRftTopologyCurve::colorForRftBranchType( branchType );
        curve->setColor( color );
        curve->setLineThickness( 3 );
        curve->setAutoCheckStateBasedOnCurveData( true );
    }

    auto curve = createAndAddCurve( track, "PRESSURE", wellName, RiaDefines::RftBranchType::RFT_ANNULUS, summaryCase );
    curve->setLineThickness( 3 );
    curve->setAutoCheckStateBasedOnCurveData( true );

    track->updateAllRequiredEditors();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::appendConnectionFactorTrack( RimWellLogPlot* plot, const QString& wellName, RimSummaryCase* summaryCase )
{
    QString resultName = "CONFAC";

    // Connection factor data can be found in the tubing, device and annulus branches.
    // Search for data ordered by annulus, device and tubing.
    RiaDefines::RftBranchType branchType = RiaDefines::RftBranchType::RFT_ANNULUS;
    bool                      foundData  = false;
    if ( RicWellLogTools::hasData( resultName, wellName, branchType, summaryCase ) )
    {
        foundData = true;
    }

    if ( !foundData && RicWellLogTools::hasData( resultName, wellName, RiaDefines::RftBranchType::RFT_DEVICE, summaryCase ) )
    {
        branchType = RiaDefines::RftBranchType::RFT_DEVICE;
        foundData  = true;
    }

    if ( !foundData && RicWellLogTools::hasData( resultName, wellName, RiaDefines::RftBranchType::RFT_TUBING, summaryCase ) )
    {
        branchType = RiaDefines::RftBranchType::RFT_TUBING;
        foundData  = true;
    }

    QString trackName = "Connection Factors";
    auto    curve     = appendTrackAndCurveForBranchType( plot, trackName, resultName, wellName, branchType, summaryCase );

    auto curveColor = cvf::Color3f( cvf::Color3f::ColorIdent::ORANGE );
    curve->setColor( curveColor );

    auto fillColor = RiaColorTools::makeLighter( curveColor, 0.3f );
    curve->setFillColor( fillColor );
    curve->setFillStyle( Qt::SolidPattern );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewRftSegmentWellLogPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create RFT Segment Plot" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
