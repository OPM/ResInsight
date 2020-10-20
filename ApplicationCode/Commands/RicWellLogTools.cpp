/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017-     Statoil ASA
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

#include "RicWellLogTools.h"

#include "RiaGuiApplication.h"
#include "RigWellLogCurveData.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimDepthTrackPlot.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogRftCurve.h"
#include "RimWellLogTrack.h"
#include "RimWellLogWbsCurve.h"
#include "RimWellMeasurementCurve.h"
#include "RimWellPath.h"

#include "RifReaderEclipseRft.h"

#include "Riu3dSelectionManager.h"
#include "RiuPlotMainWindowTools.h"

#include "WellLogCommands/RicWellLogPlotCurveFeatureImpl.h"

#include "cafSelectionManager.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSimWellInView* RicWellLogTools::selectedSimulationWell( int* branchIndex )
{
    RiuSelectionItem* selItem = Riu3dSelectionManager::instance()->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );
    RiuSimWellSelectionItem* simWellSelItem = dynamic_cast<RiuSimWellSelectionItem*>( selItem );
    if ( simWellSelItem )
    {
        ( *branchIndex ) = static_cast<int>( simWellSelItem->m_branchIndex );
        return simWellSelItem->m_simWell;
    }
    else
    {
        std::vector<RimSimWellInView*> selection;
        caf::SelectionManager::instance()->objectsByType( &selection );
        ( *branchIndex ) = 0;
        return selection.size() > 0 ? selection[0] : nullptr;
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellLogTools::wellHasRftData( const QString& wellName )
{
    RimEclipseResultCase* resultCase;
    std::vector<RimCase*> cases;
    RimProject::current()->allCases( cases );

    for ( RimCase* rimCase : cases )
    {
        if ( resultCase = dynamic_cast<RimEclipseResultCase*>( rimCase ) )
        {
            if ( resultCase->rftReader() )
            {
                return resultCase->rftReader()->wellHasRftData( wellName );
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellLogTools::isWellPathOrSimWellSelectedInView()
{
    Riu3dSelectionManager* riuSelManager = Riu3dSelectionManager::instance();
    RiuSelectionItem*      selItem       = riuSelManager->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );

    RiuSimWellSelectionItem* simWellSelectionItem = dynamic_cast<RiuSimWellSelectionItem*>( selItem );
    if ( simWellSelectionItem ) return true;

    RiuWellPathSelectionItem* wellPathSelectionItem = dynamic_cast<RiuWellPathSelectionItem*>( selItem );
    if ( wellPathSelectionItem ) return true;

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogTools::addWellLogChannelsToPlotTrack( RimWellLogTrack*                           plotTrack,
                                                     const std::vector<RimWellLogFileChannel*>& wellLogFileChannels )
{
    for ( size_t cIdx = 0; cIdx < wellLogFileChannels.size(); cIdx++ )
    {
        RimWellLogFileCurve* plotCurve = RicWellLogTools::addFileCurve( plotTrack );

        RimWellPath*    wellPath;
        RimWellLogFile* wellLogFile;
        wellLogFileChannels[cIdx]->firstAncestorOrThisOfType( wellPath );
        wellLogFileChannels[cIdx]->firstAncestorOrThisOfType( wellLogFile );

        if ( wellPath )
        {
            if ( wellLogFile ) plotCurve->setWellLogFile( wellLogFile );

            plotCurve->setWellPath( wellPath );
            plotCurve->setWellLogChannelName( wellLogFileChannels[cIdx]->name() );
            plotCurve->loadDataAndUpdate( true );
            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicWellLogTools::selectedWellPathWithLogFile()
{
    std::vector<RimWellPath*> selection;
    caf::SelectionManager::instance()->objectsByType( &selection );
    if ( selection.size() > 0 )
    {
        RimWellPath* wellPath = selection[0];
        if ( wellPath->wellLogFiles().size() > 0 )
        {
            return wellPath;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicWellLogTools::findWellPathWithLogFileFromSelection()
{
    RimWellPath* wellPath = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
    if ( wellPath->wellLogFiles().size() > 0 )
    {
        return wellPath;
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename ExtractionCurveType>
ExtractionCurveType* RicWellLogTools::addExtractionCurve( RimWellLogTrack*        plotTrack,
                                                          RimCase*                caseToApply,
                                                          Rim3dView*              view,
                                                          RimWellPath*            wellPath,
                                                          const RimSimWellInView* simWell,
                                                          int                     branchIndex,
                                                          bool                    useBranchDetection,
                                                          bool                    showPlotWindow )
{
    CVF_ASSERT( plotTrack );
    ExtractionCurveType* curve = new ExtractionCurveType();

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plotTrack->curveCount() );
    curve->setColor( curveColor );

    RimDepthTrackPlot* plot = nullptr;
    plotTrack->firstAncestorOrThisOfTypeAsserted( plot );
    RimWellLogCurveCommonDataSource* commonDataSource = plot->commonDataSource();

    if ( !caseToApply )
    {
        if ( commonDataSource->caseToApply() )
        {
            caseToApply = commonDataSource->caseToApply();
        }
        else if ( plotTrack->formationNamesCase() )
        {
            caseToApply = plotTrack->formationNamesCase();
        }
        else
        {
            std::vector<RimCase*> allCases;
            RimProject::current()->allCases( allCases );
            if ( !allCases.empty() ) caseToApply = allCases.front();
        }
    }

    QString ownerSimWellName;
    if ( !wellPath )
    {
        if ( commonDataSource->wellPathToApply() )
        {
            wellPath = commonDataSource->wellPathToApply();
        }
        else if ( plotTrack->formationWellPath() )
        {
            wellPath = plotTrack->formationWellPath();
        }
        else
        {
            auto allWellPaths = RimProject::current()->allWellPaths();
            if ( !allWellPaths.empty() )
            {
                wellPath = allWellPaths.front();
            }
        }
    }
    if ( !simWell )
    {
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( caseToApply );
        if ( eclipseCase )
        {
            if ( !commonDataSource->simWellNameToApply().isEmpty() )
            {
                ownerSimWellName = commonDataSource->simWellNameToApply();
            }
            else if ( !plotTrack->formationSimWellName().isEmpty() )
            {
                ownerSimWellName = plotTrack->formationSimWellName();
            }
            else
            {
                auto allSimWells = eclipseCase->sortedSimWellNames();
                if ( !allSimWells.empty() )
                {
                    ownerSimWellName = *allSimWells.begin();
                }
            }
        }
    }

    if ( simWell || !ownerSimWellName.isEmpty() )
    {
        QString simWellName = simWell ? simWell->name() : ownerSimWellName;
        curve->setFromSimulationWellName( simWellName, branchIndex, useBranchDetection );
    }

    if ( wellPath )
    {
        curve->setWellPath( wellPath );
        curve->setTrajectoryType( RimWellLogExtractionCurve::WELL_PATH );
    }

    if ( caseToApply )
    {
        curve->setCase( caseToApply );
    }

    if ( view )
    {
        curve->setPropertiesFromView( view );
    }

    plotTrack->addCurve( curve );

    if ( plot && curve->curveData() )
    {
        plot->setDepthUnit( curve->curveData()->depthUnit() );
    }

    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();
    plotTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RimProject::current()->updateConnectedEditors();
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );

    if ( showPlotWindow )
    {
        // Make sure the summary plot window is visible
        RiuPlotMainWindowTools::showPlotMainWindow();
    }
    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve*
    RicWellLogTools::addRftCurve( RimWellLogTrack* plotTrack, const RimSimWellInView* simWell, bool showPlotWindow )
{
    CVF_ASSERT( plotTrack );

    RimWellLogRftCurve* curve = new RimWellLogRftCurve();

    RimEclipseResultCase* resultCase = nullptr;

    std::vector<RimCase*> cases;
    RimProject::current()->allCases( cases );

    for ( RimCase* rimCase : cases )
    {
        if ( resultCase = dynamic_cast<RimEclipseResultCase*>( rimCase ) )
        {
            break;
        }
    }

    if ( simWell && resultCase )
    {
        curve->setEclipseResultCase( resultCase );
        curve->setDefaultAddress( simWell->name() );

        plotTrack->setFormationCase( resultCase );
        plotTrack->setFormationSimWellName( simWell->name() );
    }

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plotTrack->curveCount() );
    curve->setColor( curveColor );

    plotTrack->addCurve( curve );
    plotTrack->setFormationTrajectoryType( RimWellLogTrack::SIMULATION_WELL );
    plotTrack->updateConnectedEditors();

    RimProject::current()->updateConnectedEditors();
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );

    if ( showPlotWindow )
    {
        // Make sure the summary plot window is visible
        RiuPlotMainWindowTools::showPlotMainWindow();
    }

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogFileCurve* RicWellLogTools::addFileCurve( RimWellLogTrack* plotTrack, bool showPlotWindow )
{
    CVF_ASSERT( plotTrack );

    RimWellLogFileCurve* curve = new RimWellLogFileCurve();

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plotTrack->curveCount() );
    curve->setColor( curveColor );

    plotTrack->addCurve( curve );

    plotTrack->updateConnectedEditors();

    RimProject::current()->updateConnectedEditors();
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );

    if ( showPlotWindow )
    {
        // Make sure the summary plot window is visible
        RiuPlotMainWindowTools::showPlotMainWindow();
    }

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogExtractionCurve* RicWellLogTools::addWellLogExtractionCurve( RimWellLogTrack*        plotTrack,
                                                                       RimCase*                rimCase,
                                                                       Rim3dView*              view,
                                                                       RimWellPath*            wellPath,
                                                                       const RimSimWellInView* simWell,
                                                                       int                     branchIndex,
                                                                       bool                    useBranchDetection,
                                                                       bool showPlotWindow /*= true */ )
{
    return addExtractionCurve<RimWellLogExtractionCurve>( plotTrack,
                                                          rimCase,
                                                          view,
                                                          wellPath,
                                                          simWell,
                                                          branchIndex,
                                                          useBranchDetection,
                                                          showPlotWindow );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogWbsCurve* RicWellLogTools::addWellLogWbsCurve( RimWellLogTrack* plotTrack,
                                                         RimCase*         rimCase,
                                                         Rim3dView*       view,
                                                         RimWellPath*     wellPath,
                                                         int              branchIndex,
                                                         bool             useBranchDetection,
                                                         bool             showPlotWindow /*= true */ )
{
    return addExtractionCurve<RimWellLogWbsCurve>( plotTrack,
                                                   rimCase,
                                                   view,
                                                   wellPath,
                                                   nullptr,
                                                   branchIndex,
                                                   useBranchDetection,
                                                   showPlotWindow );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellMeasurementCurve* RicWellLogTools::addWellMeasurementCurve( RimWellLogTrack* plotTrack,
                                                                   RimWellPath*     wellPath,
                                                                   const QString&   measurementKind,
                                                                   bool             showPlotWindow )
{
    CVF_ASSERT( plotTrack );

    RimWellMeasurementCurve* curve = new RimWellMeasurementCurve;
    curve->setWellPath( wellPath );
    curve->setMeasurementKind( measurementKind );

    plotTrack->addCurve( curve );
    plotTrack->updateConnectedEditors();

    RimProject::current()->updateConnectedEditors();
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );

    if ( showPlotWindow )
    {
        // Make sure the summary plot window is visible
        RiuPlotMainWindowTools::showPlotMainWindow();
    }

    return curve;
}
