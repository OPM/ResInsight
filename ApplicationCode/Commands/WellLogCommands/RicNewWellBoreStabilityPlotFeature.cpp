/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2018-     Equinor ASA
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

#include "RicNewWellBoreStabilityPlotFeature.h"

#include "RicNewWellLogCurveExtractionFeature.h"
#include "RicNewWellLogFileCurveFeature.h"
#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "RigWellPath.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogFile.h"
#include "RimWellLogFileChannel.h"
#include "RimWellLogFileCurve.h"
#include "RimWellLogPlotCollection.h"
#include "RimWellLogTrack.h"
#include "RimWellLogWbsCurve.h"
#include "RimWellPath.h"

#include "RicWellLogTools.h"
#include "RiuPlotMainWindowTools.h"

#include "RiaApplication.h"

#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"
#include "cvfMath.h"

#include <QAction>
#include <QDateTime>
#include <QString>

#include <algorithm>

CAF_CMD_SOURCE_INIT( RicNewWellBoreStabilityPlotFeature, "RicNewWellBoreStabilityPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellBoreStabilityPlotFeature::isCommandEnabled()
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view ) return false;
    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( view );
    return geoMechView != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::onActionTriggered( bool isChecked )
{
    RimWellPath*              wellPath = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
    RimWellLogPlotCollection* plotCollection = caf::SelectionManager::instance()
                                                   ->selectedItemOfType<RimWellLogPlotCollection>();
    if ( !wellPath )
    {
        if ( plotCollection )
        {
            RimProject* project = nullptr;
            plotCollection->firstAncestorOrThisOfTypeAsserted( project );
            std::vector<RimWellPath*> allWellPaths;
            project->descendantsIncludingThisOfType( allWellPaths );
            if ( !allWellPaths.empty() )
            {
                wellPath = allWellPaths.front();
            }
        }
    }

    if ( !wellPath )
    {
        return;
    }

    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view ) return;

    RimGeoMechView* geoMechView = dynamic_cast<RimGeoMechView*>( view );
    if ( !geoMechView ) return;

    caf::ProgressInfo progInfo( 100, "Creating Well Bore Stability Plot" );

    RimGeoMechCase*           geoMechCase = geoMechView->geoMechCase();
    RimWellBoreStabilityPlot* plot        = RicNewWellLogPlotFeatureImpl::createWellBoreStabilityPlot( false,
                                                                                                "Well Bore Stability" );

    {
        auto task = progInfo.task( "Creating formation track", 2 );
        createFormationTrack( plot, wellPath, geoMechCase );
    }

    {
        auto task = progInfo.task( "Creating well design track", 3 );
        createCasingShoeTrack( plot, wellPath, geoMechCase );
    }

    {
        auto task = progInfo.task( "Creating stability curves track", 75 );
        createStabilityCurvesTrack( plot, wellPath, geoMechView );
    }

    {
        auto task = progInfo.task( "Creating angles track", 15 );
        createAnglesTrack( plot, wellPath, geoMechView );
    }

    {
        auto task = progInfo.task( "Updating all tracks", 5 );

        plot->enableAllAutoNameTags( true );
        plot->setPlotTitleVisible( true );
        plot->setTrackLegendsVisible( true );
        plot->setTrackLegendsHorizontal( true );
        plot->setDepthType( RimWellLogPlot::TRUE_VERTICAL_DEPTH );
        plot->setDepthAutoZoom( true );

        RicNewWellLogPlotFeatureImpl::updateAfterCreation( plot );
    }

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );

    // Make sure the summary plot window is visible
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Well Bore Stability Plot" );
    actionToSetup->setIcon( QIcon( ":/WellLogPlot16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createFormationTrack( RimWellBoreStabilityPlot* plot,
                                                               RimWellPath*              wellPath,
                                                               RimGeoMechCase*           geoMechCase )
{
    RimWellLogTrack* formationTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Formations", plot );
    formationTrack->setFormationWellPath( wellPath );
    formationTrack->setFormationCase( geoMechCase );
    formationTrack->setAnnotationType( RiuPlotAnnotationTool::FORMATION_ANNOTATIONS );
    formationTrack->setVisibleXRange( 0.0, 0.0 );
    formationTrack->setWidthScaleFactor( RimWellLogTrack::NARROW_TRACK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createCasingShoeTrack( RimWellBoreStabilityPlot* plot,
                                                                RimWellPath*              wellPath,
                                                                RimGeoMechCase*           geoMechCase )
{
    RimWellLogTrack* casingShoeTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Well Design", plot );
    casingShoeTrack->setWidthScaleFactor( RimWellLogTrack::NARROW_TRACK );
    casingShoeTrack->setFormationWellPath( wellPath );
    casingShoeTrack->setFormationCase( geoMechCase );
    casingShoeTrack->setAnnotationType( RiuPlotAnnotationTool::FORMATION_ANNOTATIONS );
    casingShoeTrack->setAnnotationDisplay( RiuPlotAnnotationTool::DARK_LINES );
    casingShoeTrack->setShowRegionLabels( false );
    casingShoeTrack->setShowWellPathAttributes( true );
    casingShoeTrack->setWellPathAttributesSource( wellPath );
    casingShoeTrack->setVisibleXRange( 0.0, 0.0 );
    casingShoeTrack->setAutoScaleXEnabled( true );
    casingShoeTrack->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createStabilityCurvesTrack( RimWellBoreStabilityPlot* plot,
                                                                     RimWellPath*              wellPath,
                                                                     RimGeoMechView*           geoMechView )
{
    RimWellLogTrack* stabilityCurvesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false,
                                                                                                  "Stability Curves",
                                                                                                  plot );
    stabilityCurvesTrack->setWidthScaleFactor( RimWellLogTrack::EXTRA_WIDE_TRACK );
    stabilityCurvesTrack->setAutoScaleXEnabled( true );
    stabilityCurvesTrack->setTickIntervals( 0.5, 0.05 );
    stabilityCurvesTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    stabilityCurvesTrack->setFormationWellPath( wellPath );
    stabilityCurvesTrack->setFormationCase( geoMechView->geoMechCase() );
    stabilityCurvesTrack->setAnnotationType( RiuPlotAnnotationTool::NO_ANNOTATIONS );
    stabilityCurvesTrack->setShowRegionLabels( true );

    std::vector<QString> resultNames = RiaDefines::wellPathStabilityResultNames();

    std::vector<cvf::Color3f> colors = {cvf::Color3f::BLUE,
                                        cvf::Color3f::BROWN,
                                        cvf::Color3f::RED,
                                        cvf::Color3f::PURPLE,
                                        cvf::Color3f::DARK_GREEN};

    for ( size_t i = 0; i < resultNames.size(); ++i )
    {
        const QString&      resultName = resultNames[i];
        RigFemResultAddress resAddr( RIG_WELLPATH_DERIVED, resultName.toStdString(), "" );
        RimWellLogWbsCurve* curve = RicWellLogTools::addWellLogWbsCurve( stabilityCurvesTrack,
                                                                         geoMechView,
                                                                         wellPath,
                                                                         nullptr,
                                                                         -1,
                                                                         false,
                                                                         false );
        curve->setGeoMechResultAddress( resAddr );
        curve->setCurrentTimeStep( geoMechView->currentTimeStep() );
        curve->setCustomName( resultName );
        curve->setColor( colors[i % colors.size()] );
        curve->setLineThickness( 2 );
        curve->loadDataAndUpdate( false );
        curve->setSmoothCurve( true );
        curve->setSmoothingThreshold( 0.002 );
    }
    stabilityCurvesTrack->calculateXZoomRangeAndUpdateQwt();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createAnglesTrack( RimWellBoreStabilityPlot* plot,
                                                            RimWellPath*              wellPath,
                                                            RimGeoMechView*           geoMechView )
{
    RimWellLogTrack*     wellPathAnglesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false,
                                                                                                 "Well Path Angles",
                                                                                                 plot );
    double               minValue = 360.0, maxValue = 0.0;
    const double         angleIncrement = 90.0;
    std::vector<QString> resultNames    = RiaDefines::wellPathAngleResultNames();

    std::vector<cvf::Color3f> colors = {cvf::Color3f::GREEN, cvf::Color3f::ORANGE};

    std::vector<RiuQwtPlotCurve::LineStyleEnum> lineStyles = {RiuQwtPlotCurve::STYLE_SOLID, RiuQwtPlotCurve::STYLE_DASH};

    for ( size_t i = 0; i < resultNames.size(); ++i )
    {
        const QString&             resultName = resultNames[i];
        RigFemResultAddress        resAddr( RIG_WELLPATH_DERIVED, resultName.toStdString(), "" );
        RimWellLogExtractionCurve* curve = RicWellLogTools::addWellLogExtractionCurve( wellPathAnglesTrack,
                                                                                       geoMechView,
                                                                                       wellPath,
                                                                                       nullptr,
                                                                                       -1,
                                                                                       false,
                                                                                       false );
        curve->setGeoMechResultAddress( resAddr );
        curve->setCurrentTimeStep( geoMechView->currentTimeStep() );
        curve->setCustomName( resultName );

        curve->setColor( colors[i % colors.size()] );
        curve->setLineStyle( lineStyles[i % lineStyles.size()] );
        curve->setLineThickness( 2 );

        curve->loadDataAndUpdate( false );

        double actualMinValue = minValue, actualMaxValue = maxValue;
        curve->xValueRangeInQwt( &actualMinValue, &actualMaxValue );
        while ( maxValue < actualMaxValue )
        {
            maxValue += angleIncrement;
        }
        while ( minValue > actualMinValue )
        {
            minValue -= angleIncrement;
        }
        maxValue = cvf::Math::clamp( maxValue, angleIncrement, 360.0 );
        minValue = cvf::Math::clamp( minValue, 0.0, maxValue - 90.0 );
    }
    wellPathAnglesTrack->setWidthScaleFactor( RimWellLogTrack::NORMAL_TRACK );
    wellPathAnglesTrack->setVisibleXRange( minValue, maxValue );
    wellPathAnglesTrack->setTickIntervals( 90.0, 30.0 );
    wellPathAnglesTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    wellPathAnglesTrack->setFormationWellPath( wellPath );
    wellPathAnglesTrack->setFormationCase( geoMechView->geoMechCase() );
    wellPathAnglesTrack->setAnnotationType( RiuPlotAnnotationTool::NO_ANNOTATIONS );
    wellPathAnglesTrack->setShowRegionLabels( false );
}
