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
#include "RiaRftDefines.h"

#include "RifReaderOpmRft.h"

#include "RimRftCase.h"
#include "RimRftTopologyCurve.h"
#include "RimSummaryCase.h"
#include "RimWellLogPlot.h"
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

    auto plot = RicNewWellLogPlotFeatureImpl::createHorizontalWellLogPlot();
    plot->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );
    QString templateText = RiaDefines::namingVariableCase() + ", " + RiaDefines::namingVariableWell() + " - Branch " +
                           RiaDefines::namingVariableWellBranch() + ", " + RiaDefines::namingVariableTime();
    plot->setNameTemplateText( templateText );
    plot->setPlotTitleVisible( true );
    plot->setLegendsClickable( false );

    QString wellName = "Unknown";

    auto rftReader = summaryCase->rftReader();
    if ( rftReader )
    {
        auto wellNames = rftReader->wellNames();
        if ( !wellNames.empty() ) wellName = *wellNames.begin();
    }

    std::vector<QString> resultNames = { "SEGGRAT", "SEGORAT", "SEGWRAT" };

    std::vector<RiaDefines::RftBranchType> branchTypes{ RiaDefines::RftBranchType::RFT_ANNULUS,
                                                        RiaDefines::RftBranchType::RFT_DEVICE,
                                                        RiaDefines::RftBranchType::RFT_TUBING };

    for ( auto branchType : branchTypes )
    {
        appendTrackAndCurveForBranchType( plot, resultNames, wellName, branchType, summaryCase );
    }

    RicNewRftSegmentWellLogPlotFeature::appendTopologyTrack( plot, wellName, summaryCase );

    plot->loadDataAndUpdate();

    RiuPlotMainWindowTools::onObjectAppended( plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewMultiPhaseRftSegmentPlotFeature::appendTrackAndCurveForBranchType( RimWellLogPlot*             plot,
                                                                              const std::vector<QString>& resultNames,
                                                                              const QString&              wellName,
                                                                              RiaDefines::RftBranchType   branchType,
                                                                              RimSummaryCase*             summaryCase )
{
    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addPlot( plotTrack );
    plotTrack->setDescription( QString( "Track %1" ).arg( plot->plotCount() ) );

    plot->loadDataAndUpdate();

    for ( const auto& resultName : resultNames )
    {
        auto curve = RicWellLogTools::addSummaryRftSegmentCurve( plotTrack, resultName, wellName, branchType, summaryCase );
        curve->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );

        QString templateText = RiaDefines::namingVariableResultName() + ", " + RiaDefines::namingVariableResultType();
        curve->setCurveNameTemplateText( templateText );

        curve->setIsStacked( true );
        curve->loadDataAndUpdate( true );

        curve->updateAllRequiredEditors();
        RiuPlotMainWindowTools::setExpanded( curve );
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
