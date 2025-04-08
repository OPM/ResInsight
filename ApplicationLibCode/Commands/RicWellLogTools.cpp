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

#include "RigEclipseCaseData.h"
#include "Well/RigWellLogCurveData.h"

#include "Rim3dView.h"
#include "RimCase.h"
#include "RimDepthTrackPlot.h"
#include "RimEclipseResultCase.h"
#include "RimProject.h"
#include "RimSimWellInView.h"
#include "RimSummaryCase.h"
#include "RimWellLogCalculatedCurve.h"
#include "RimWellLogChannel.h"
#include "RimWellLogCurveCommonDataSource.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogLasFileCurve.h"
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
    RiuSelectionItem*        selItem        = Riu3dSelectionManager::instance()->selectedItem( Riu3dSelectionManager::RUI_TEMPORARY );
    RiuSimWellSelectionItem* simWellSelItem = dynamic_cast<RiuSimWellSelectionItem*>( selItem );
    if ( simWellSelItem )
    {
        ( *branchIndex ) = static_cast<int>( simWellSelItem->m_branchIndex );
        return simWellSelItem->m_simWell;
    }
    else
    {
        ( *branchIndex ) = 0;

        return caf::SelectionManager::instance()->selectedItemOfType<RimSimWellInView>();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellLogTools::hasRftData()
{
    RimEclipseResultCase* resultCase;
    std::vector<RimCase*> cases = RimProject::current()->allGridCases();
    for ( RimCase* rimCase : cases )
    {
        if ( ( resultCase = dynamic_cast<RimEclipseResultCase*>( rimCase ) ) )
        {
            if ( resultCase->rftReader() )
            {
                return true;
            }
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellLogTools::hasRftDataForWell( const QString& wellName )
{
    RimEclipseResultCase* resultCase;
    std::vector<RimCase*> cases = RimProject::current()->allGridCases();
    for ( RimCase* rimCase : cases )
    {
        if ( ( resultCase = dynamic_cast<RimEclipseResultCase*>( rimCase ) ) )
        {
            if ( resultCase->rftReader() )
            {
                auto wellNames = resultCase->rftReader()->wellNames();
                for ( const auto& w : wellNames )
                {
                    if ( w == wellName ) return true;
                }
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
    return wellPathSelectionItem != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicWellLogTools::addWellLogChannelsToPlotTrack( RimWellLogTrack* plotTrack, const std::vector<RimWellLogChannel*>& wellLogChannels )
{
    for ( RimWellLogChannel* wellLogChannel : wellLogChannels )
    {
        RimWellLogLasFileCurve* plotCurve = RicWellLogTools::addFileCurve( plotTrack );

        RimWellPath*       wellPath    = wellLogChannel->firstAncestorOrThisOfType<RimWellPath>();
        RimWellLogLasFile* wellLogFile = wellLogChannel->firstAncestorOrThisOfType<RimWellLogLasFile>();

        if ( wellPath )
        {
            if ( wellLogFile ) plotCurve->setWellLog( wellLogFile );

            plotCurve->setWellPath( wellPath );
            plotCurve->setWellLogChannelName( wellLogChannel->name() );
            plotCurve->loadDataAndUpdate( true );
            plotCurve->updateConnectedEditors();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicWellLogTools::selectedWellPathWithLog()
{
    const auto selection = caf::SelectionManager::instance()->objectsByType<RimWellPath>();

    for ( const auto& wellPath : selection )
    {
        if ( !wellPath->wellLogs().empty() )
        {
            return wellPath;
        }
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellPath* RicWellLogTools::findWellPathWithLogFromSelection()
{
    RimWellPath* wellPath = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
    if ( wellPath && !wellPath->wellLogs().empty() )
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

    RiaDefines::DepthUnitType defaultDepthUnit = RiaDefines::DepthUnitType::UNIT_METER;

    if ( auto eclipseCase = dynamic_cast<RimEclipseCase*>( caseToApply ) )
    {
        defaultDepthUnit = RiaDefines::fromEclipseUnit( eclipseCase->eclipseCaseData()->unitsType() );
    }

    ExtractionCurveType* curve = new ExtractionCurveType();
    curve->setDepthUnit( defaultDepthUnit );

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plotTrack->curveCount() );
    curve->setColor( curveColor );

    RimDepthTrackPlot*               plot             = plotTrack->firstAncestorOrThisOfTypeAsserted<RimDepthTrackPlot>();
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
            std::vector<RimCase*> allCases = RimProject::current()->allGridCases();
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
RimWellLogRftCurve* RicWellLogTools::addRftCurve( RimWellLogTrack* plotTrack, const RimSimWellInView* simWell, bool showPlotWindow )
{
    CVF_ASSERT( plotTrack );

    RimWellLogRftCurve* curve = new RimWellLogRftCurve();

    RimEclipseResultCase* resultCase = nullptr;

    std::vector<RimCase*> cases = RimProject::current()->allGridCases();
    for ( RimCase* rimCase : cases )
    {
        if ( ( resultCase = dynamic_cast<RimEclipseResultCase*>( rimCase ) ) )
        {
            break;
        }
    }

    if ( simWell && resultCase )
    {
        curve->setEclipseCase( resultCase );
        curve->setDefaultAddress( simWell->name() );

        plotTrack->setFormationCase( resultCase );
        plotTrack->setFormationSimWellName( simWell->name() );
    }
    else if ( resultCase )
    {
        curve->setEclipseCase( resultCase );

        auto wellNames = resultCase->rftReader()->wellNames();
        if ( !wellNames.empty() )
        {
            auto wellName = *( wellNames.begin() );
            curve->setDefaultAddress( wellName );
        }
    }
    else
    {
        auto sumCases = RimProject::current()->allSummaryCases();

        for ( auto sc : sumCases )
        {
            if ( sc->rftReader() )
            {
                auto rftReader = sc->rftReader();

                curve->setSummaryCase( sc );

                auto addresses = rftReader->eclipseRftAddresses();
                if ( !addresses.empty() ) curve->setRftAddress( *addresses.begin() );
            }
        }
    }

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plotTrack->curveCount() );
    curve->setColor( curveColor );

    plotTrack->addCurve( curve );
    plotTrack->setFormationTrajectoryType( RimWellLogTrack::SIMULATION_WELL );
    plotTrack->updateConnectedEditors();

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
RimWellLogLasFileCurve* RicWellLogTools::addFileCurve( RimWellLogTrack* plotTrack, bool showPlotWindow )
{
    CVF_ASSERT( plotTrack );

    RimWellLogLasFileCurve* curve = new RimWellLogLasFileCurve();

    cvf::Color3f curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plotTrack->curveCount() );
    curve->setColor( curveColor );

    plotTrack->addCurve( curve );

    plotTrack->updateConnectedEditors();

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
                                                                       bool                    showPlotWindow /*= true */ )
{
    return addExtractionCurve<RimWellLogExtractionCurve>( plotTrack, rimCase, view, wellPath, simWell, branchIndex, useBranchDetection, showPlotWindow );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogCurve* RicWellLogTools::addSummaryRftCurve( RimWellLogTrack* plotTrack, RimSummaryCase* rimCase )
{
    auto curve = new RimWellLogRftCurve();

    curve->setSummaryCase( rimCase );
    auto rftReader = rimCase->rftReader();
    if ( rftReader )
    {
        QString wellName;
        auto    wellNames = rftReader->wellNames();
        if ( !wellNames.empty() ) wellName = *wellNames.begin();

        QDateTime dateTime;

        auto timeSteps = rftReader->availableTimeSteps( wellName );
        if ( !timeSteps.empty() ) dateTime = *timeSteps.rbegin();

        RifEclipseRftAddress adr =
            RifEclipseRftAddress::createAddress( wellName, dateTime, RifEclipseRftAddress::RftWellLogChannelType::PRESSURE );
        curve->setRftAddress( adr );
    }

    plotTrack->addCurve( curve );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellLogRftCurve* RicWellLogTools::addSummaryRftSegmentCurve( RimWellLogTrack*          plotTrack,
                                                                const QString&            resultName,
                                                                const QString&            wellName,
                                                                RiaDefines::RftBranchType branchType,
                                                                RimSummaryCase*           rimCase )
{
    auto curve = new RimWellLogRftCurve();

    curve->setSummaryCase( rimCase );

    auto rftReader = rimCase->rftReader();
    if ( rftReader )
    {
        QDateTime dateTime;

        auto timeSteps = rftReader->availableTimeSteps( wellName );
        if ( !timeSteps.empty() ) dateTime = *timeSteps.rbegin();

        RifEclipseRftAddress adr = RifEclipseRftAddress::createBranchSegmentAddress( wellName, dateTime, resultName, 1, branchType );
        curve->setRftAddress( adr );
        curve->assignColorFromResultName( resultName );
        curve->setLineThickness( 4 );

        curve->setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT );
    }

    plotTrack->addCurve( curve );

    return curve;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicWellLogTools::hasData( const QString& resultName, const QString& wellName, RiaDefines::RftBranchType branchType, RimSummaryCase* rimCase )
{
    auto rftReader = rimCase->rftReader();
    if ( !rftReader ) return false;

    QDateTime dateTime;

    auto timeSteps = rftReader->availableTimeSteps( wellName );
    if ( !timeSteps.empty() ) dateTime = *timeSteps.rbegin();

    RifEclipseRftAddress adr = RifEclipseRftAddress::createBranchSegmentAddress( wellName, dateTime, resultName, 1, branchType );

    std::vector<double> values;
    rftReader->values( adr, &values );

    return !values.empty();
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
    return addExtractionCurve<RimWellLogWbsCurve>( plotTrack, rimCase, view, wellPath, nullptr, branchIndex, useBranchDetection, showPlotWindow );
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
RimWellLogCalculatedCurve* RicWellLogTools::addWellLogCalculatedCurve( RimWellLogTrack* plotTrack, bool showPlotWindow )
{
    CVF_ASSERT( plotTrack );

    RimWellLogCalculatedCurve* curve      = new RimWellLogCalculatedCurve();
    const cvf::Color3f         curveColor = RicWellLogPlotCurveFeatureImpl::curveColorFromTable( plotTrack->curveCount() );
    curve->setColor( curveColor );

    plotTrack->addCurve( curve );
    plotTrack->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );

    if ( showPlotWindow )
    {
        // Make sure the summary plot window is visible
        RiuPlotMainWindowTools::showPlotMainWindow();
    }

    return curve;
}
