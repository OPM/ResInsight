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

#include "RifReaderOpmRft.h"

#include "RimRftCase.h"
#include "RimRftTopologyCurve.h"
#include "RimSummaryCase.h"
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

    auto plot = RicNewWellLogPlotFeatureImpl::createHorizontalWellLogPlot();
    plot->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );
    QString templateText = RiaDefines::namingVariableCase() + ", " + RiaDefines::namingVariableWell() + " - Branch " +
                           RiaDefines::namingVariableWellBranch() + ", " + RiaDefines::namingVariableTime();
    plot->setNameTemplateText( templateText );
    plot->setPlotTitleVisible( true );
    plot->setLegendItemsClickable( false );
    plot->enableDepthMarkerLine( true );

    QString wellName = "Unknown";

    auto rftReader = summaryCase->rftReader();
    if ( rftReader )
    {
        auto wellNames = rftReader->wellNames();
        if ( !wellNames.empty() ) wellName = *wellNames.begin();
    }

    QString resultName = "SEGGRAT";

    std::vector<RiaDefines::RftBranchType> branchTypes{ RiaDefines::RftBranchType::RFT_ANNULUS,
                                                        RiaDefines::RftBranchType::RFT_DEVICE,
                                                        RiaDefines::RftBranchType::RFT_TUBING };

    for ( auto branchType : branchTypes )
    {
        appendTrackAndCurveForBranchType( plot, resultName, wellName, branchType, summaryCase );
    }

    appendTopologyTrack( plot, wellName, summaryCase );
    plot->loadDataAndUpdate();

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
    curve->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );

    QString templateText = RiaDefines::namingVariableResultName() + ", " + RiaDefines::namingVariableResultType();
    curve->setCurveNameTemplateText( templateText );

    curve->loadDataAndUpdate( true );

    curve->updateAllRequiredEditors();
    RiuPlotMainWindowTools::setExpanded( curve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimPlotCurve*> RicNewRftSegmentWellLogPlotFeature::appendAdditionalDataSourceTrack( RimWellLogPlot* plot,
                                                                                                const QString& wellName,
                                                                                                RimSummaryCase* summaryCase )
{
    auto additionalDataSourceTrack = new RimWellLogTrack();
    additionalDataSourceTrack->setDescription( "Additional Data Source Curves" );

    plot->addPlot( additionalDataSourceTrack );
    additionalDataSourceTrack->setShowWindow( false );

    QString resultName   = RiaDefines::segmentNumberResultName();
    QString templateText = RiaDefines::namingVariableResultName() + ", " + RiaDefines::namingVariableResultType();

    std::vector<RimPlotCurve*> curves;

    for ( auto branchType : { RiaDefines::RftBranchType::RFT_TUBING,
                              RiaDefines::RftBranchType::RFT_DEVICE,
                              RiaDefines::RftBranchType::RFT_ANNULUS } )
    {
        auto curve = RicWellLogTools::addSummaryRftSegmentCurve( additionalDataSourceTrack,
                                                                 resultName,
                                                                 wellName,
                                                                 branchType,
                                                                 summaryCase );
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
void RicNewRftSegmentWellLogPlotFeature::appendTopologyTrack( RimWellLogPlot* plot,
                                                              const QString&  wellName,
                                                              RimSummaryCase* summaryCase )
{
    QDateTime dateTime;
    int       branchIndex = 1;

    auto rftReader = dynamic_cast<RifReaderOpmRft*>( summaryCase->rftReader() );
    if ( rftReader )
    {
        auto timeSteps = rftReader->availableTimeSteps( wellName );
        if ( !timeSteps.empty() ) dateTime = *timeSteps.rbegin();
    }

    auto track = new RimWellLogTrack();
    track->setDescription( "Topology" );
    track->enablePropertyAxis( false );

    plot->addPlot( track );

    std::vector<RimPlotCurve*> additionalDataSourceCurves = appendAdditionalDataSourceTrack( plot, wellName, summaryCase );

    for ( auto branchType : { RiaDefines::RftBranchType::RFT_TUBING,
                              RiaDefines::RftBranchType::RFT_DEVICE,
                              RiaDefines::RftBranchType::RFT_ANNULUS } )
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
void RicNewRftSegmentWellLogPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "Create RFT Segment Plot" );
    actionToSetup->setIcon( QIcon( ":/WellLogCurve16x16.png" ) );
}
