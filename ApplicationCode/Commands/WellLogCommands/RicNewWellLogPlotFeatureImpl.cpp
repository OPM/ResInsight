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

#include "RicNewWellLogPlotFeatureImpl.h"

#include "RimCase.h"
#include "RimEclipseCase.h"
#include "RimStimPlanModelPlot.h"
#include "RimStimPlanModelPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"

#include "RiaGuiApplication.h"

#include "cvfAssert.h"

#include <QString>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellBoreStabilityPlot*
    RicNewWellLogPlotFeatureImpl::createWellBoreStabilityPlot( bool           showAfterCreation /*= true*/,
                                                               const QString& plotDescription /*= QString("")*/,
                                                               const RimWbsParameters* params /*= nullptr*/ )
{
    RimWellLogPlotCollection* wellLogPlotColl = wellLogPlotCollection();
    CVF_ASSERT( wellLogPlotColl );

    // Make sure the summary plot window is created
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();

    RimWellBoreStabilityPlot* plot = new RimWellBoreStabilityPlot();
    if ( params )
    {
        plot->copyWbsParameters( params );
    }

    plot->setAsPlotMdiWindow();

    wellLogPlotColl->addWellLogPlot( plot );

    if ( !plotDescription.isEmpty() )
    {
        plot->nameConfig()->setCustomName( plotDescription );
    }
    else
    {
        plot->nameConfig()->setCustomName(
            QString( "Well Bore Stability Plot %1" ).arg( wellLogPlotCollection()->wellLogPlots().size() ) );
    }

    if ( showAfterCreation )
    {
        RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    }

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlot* RicNewWellLogPlotFeatureImpl::createWellLogPlot( bool showAfterCreation, const QString& plotDescription )
{
    RimWellLogPlotCollection* wellLogPlotColl = wellLogPlotCollection();
    CVF_ASSERT( wellLogPlotColl );

    // Make sure the summary plot window is created
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();

    RimWellLogPlot* plot = new RimWellLogPlot();
    plot->setAsPlotMdiWindow();

    wellLogPlotColl->addWellLogPlot( plot );

    if ( !plotDescription.isEmpty() )
    {
        plot->nameConfig()->setCustomName( plotDescription );
    }
    else
    {
        plot->nameConfig()->setCustomName(
            QString( "Well Log Plot %1" ).arg( wellLogPlotCollection()->wellLogPlots().size() ) );
    }

    if ( showAfterCreation )
    {
        RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    }

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogTrack* RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( bool               updateAfter,
                                                                       const QString&     trackDescription,
                                                                       RimDepthTrackPlot* existingPlot )
{
    RimDepthTrackPlot* plot = existingPlot;
    if ( plot == nullptr )
    {
        plot = createWellLogPlot();
    }

    RimCase*     caseToApply     = nullptr;
    RimWellPath* wellPathToApply = nullptr;
    QString      simWellToApply;

    RimWellLogCurveCommonDataSource* commonDataSource = plot->commonDataSource();
    caseToApply                                       = commonDataSource->caseToApply();
    wellPathToApply                                   = commonDataSource->wellPathToApply();
    simWellToApply                                    = commonDataSource->simWellNameToApply();
    caf::Tristate branchDetectionToApply              = commonDataSource->branchDetectionToApply();
    int           branchIndexToApply                  = commonDataSource->branchIndexToApply();

    if ( !caseToApply )
    {
        std::vector<RimCase*> allCases;
        RimProject::current()->allCases( allCases );
        if ( !allCases.empty() )
        {
            caseToApply = allCases.front();
        }
    }

    if ( !wellPathToApply && caseToApply )
    {
        auto allWellPaths = RimProject::current()->allWellPaths();
        if ( !allWellPaths.empty() )
        {
            wellPathToApply = allWellPaths.front();
        }
    }
    if ( simWellToApply.isEmpty() && caseToApply )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( caseToApply );
        if ( eclipseCase )
        {
            auto allSimWells = eclipseCase->sortedSimWellNames();
            if ( !allSimWells.empty() )
            {
                simWellToApply = *allSimWells.begin();
            }
        }
    }

    RimWellLogTrack* plotTrack = new RimWellLogTrack();
    plot->addPlot( plotTrack );
    if ( !trackDescription.isEmpty() )
    {
        plotTrack->setDescription( trackDescription );
    }
    else
    {
        plotTrack->setDescription( QString( "Track %1" ).arg( plot->plotCount() ) );
    }

    if ( caseToApply )
    {
        plotTrack->setFormationCase( caseToApply );
    }

    if ( wellPathToApply )
    {
        plotTrack->setFormationWellPath( wellPathToApply );
    }

    if ( !simWellToApply.isEmpty() )
    {
        plotTrack->setFormationSimWellName( simWellToApply );
    }

    if ( wellPathToApply )
    {
        plotTrack->setFormationTrajectoryType( RimWellLogTrack::WELL_PATH );
    }
    else if ( !simWellToApply.isEmpty() )
    {
        plotTrack->setFormationTrajectoryType( RimWellLogTrack::SIMULATION_WELL );
    }

    if ( !branchDetectionToApply.isPartiallyTrue() )
    {
        plotTrack->setFormationBranchDetection( branchDetectionToApply.isTrue() );
    }

    if ( branchIndexToApply >= 0 )
    {
        plotTrack->setFormationBranchIndex( branchIndexToApply );
    }

    if ( updateAfter )
    {
        updateAfterCreation( plot );
    }

    return plotTrack;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellLogPlotFeatureImpl::updateAfterCreation( RimDepthTrackPlot* plot )
{
    CVF_ASSERT( plot );
    plot->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogPlotCollection* RicNewWellLogPlotFeatureImpl::wellLogPlotCollection()
{
    RimProject* project = RimProject::current();
    CVF_ASSERT( project );

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT( mainPlotColl );

    RimWellLogPlotCollection* wellLogPlotColl = mainPlotColl->wellLogPlotCollection();
    CVF_ASSERT( wellLogPlotColl );

    return mainPlotColl->wellLogPlotCollection();
}
