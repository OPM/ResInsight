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

#include "RiaColorTables.h"
#include "RiaLogging.h"
#include "RiaResultNames.h"

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
#include "RimWellMeasurement.h"
#include "RimWellMeasurementCollection.h"
#include "RimWellMeasurementCurve.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

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
RimWellBoreStabilityPlot*
    RicNewWellBoreStabilityPlotFeature::createPlot( RimGeoMechCase*         geoMechCase,
                                                    RimWellPath*            wellPath,
                                                    int                     timeStep,
                                                    const RimWbsParameters* parameters /* = nullptr*/ )
{
    caf::ProgressInfo progInfo( 100, "Creating Well Bore Stability Plot" );

    RimWellBoreStabilityPlot* plot =
        RicNewWellLogPlotFeatureImpl::createWellBoreStabilityPlot( false, "Well Bore Stability", parameters );
    plot->setCaseWellPathAndTimeStep( geoMechCase, wellPath, timeStep );

    {
        auto task = progInfo.task( "Creating formation track", 2 );
        createFormationTrack( plot, wellPath, geoMechCase );
    }

    {
        auto task = progInfo.task( "Creating well design track", 3 );
        createCasingShoeTrack( plot, wellPath, geoMechCase );
    }

    {
        auto task = progInfo.task( "Creating parameters track", 15 );
        createParametersTrack( plot, wellPath, geoMechCase, timeStep );
    }

    {
        auto task = progInfo.task( "Creating stability curves track", 60 );
        createStabilityCurvesTrack( plot, wellPath, geoMechCase, timeStep );
    }

    {
        auto task = progInfo.task( "Creating angles track", 15 );
        createAnglesTrack( plot, wellPath, geoMechCase, timeStep );
    }

    {
        auto task = progInfo.task( "Updating all tracks", 5 );

        plot->nameConfig()->setAutoNameTags( true, true, true, true, true );
        plot->setPlotTitleVisible( true );
        plot->setLegendsVisible( true );
        plot->setLegendsHorizontal( true );
        plot->setDepthType( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB );
        plot->setAutoScaleDepthEnabled( true );

        RicNewWellLogPlotFeatureImpl::updateAfterCreation( plot );
    }

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );

    // Make sure the summary plot window is visible
    RiuPlotMainWindowTools::showPlotMainWindow();

    return plot;
}

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
    RimWellLogPlotCollection* plotCollection =
        caf::SelectionManager::instance()->selectedItemOfType<RimWellLogPlotCollection>();
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

    RimGeoMechCase* geoMechCase = geoMechView->geoMechCase();

    if ( !geoMechCase )
    {
        return;
    }

    auto wellPathGeometry = wellPath->wellPathGeometry();

    if ( !wellPathGeometry )
    {
        RiaLogging::error(
            QString( "The well path %1 has no geometry. Cannot create a Well Bore Stability Plot" ).arg( wellPath->name() ) );
        return;
    }
    if ( wellPathGeometry->rkbDiff() == HUGE_VAL )
    {
        RiaLogging::error(
            QString( "The well path %1 has no datum elevation and we cannot estimate TVDRKB. Cannot "
                     "create a Well Bore Stability Plot\nTo solve this issue, please activate the Property Editor, "
                     "select Well Targets and activate Generate Well Target at Sea Level" )
                .arg( wellPath->name() ) );
        return;
    }

    createPlot( geoMechCase, wellPath, view->currentTimeStep() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Well Bore Stability Plot" );
    actionToSetup->setIcon( QIcon( ":/WellBoreStability16x16.png" ) );
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
    formationTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS );
    formationTrack->setVisibleXRange( 0.0, 0.0 );
    formationTrack->setColSpan( RimPlot::ONE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createCasingShoeTrack( RimWellBoreStabilityPlot* plot,
                                                                RimWellPath*              wellPath,
                                                                RimGeoMechCase*           geoMechCase )
{
    RimWellLogTrack* casingShoeTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Well Design", plot );
    casingShoeTrack->setColSpan( RimPlot::ONE );
    casingShoeTrack->setFormationWellPath( wellPath );
    casingShoeTrack->setFormationCase( geoMechCase );
    casingShoeTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS );
    casingShoeTrack->setAnnotationDisplay( RiuPlotAnnotationTool::COLOR_SHADING_AND_LINES );
    casingShoeTrack->setShowRegionLabels( false );
    casingShoeTrack->setShowWellPathAttributes( true );
    casingShoeTrack->setShowBothSidesOfWell( false );
    casingShoeTrack->setAnnotationTransparency( 90 );
    casingShoeTrack->setWellPathAttributesSource( wellPath );
    casingShoeTrack->setVisibleXRange( 0.0, 0.0 );
    casingShoeTrack->setAutoScaleXEnabled( true );
    casingShoeTrack->loadDataAndUpdate();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createParametersTrack( RimWellBoreStabilityPlot* plot,
                                                                RimWellPath*              wellPath,
                                                                RimGeoMechCase*           geoMechCase,
                                                                int                       timeStep )
{
    RimWellLogTrack* paramCurvesTrack =
        RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "WBS Parameters", plot );
    paramCurvesTrack->setColSpan( RimPlot::TWO );
    paramCurvesTrack->setVisibleXRange( 0.0, 2.0 );
    paramCurvesTrack->setAutoScaleXEnabled( true );
    paramCurvesTrack->setTickIntervals( 1.0, 0.2 );
    paramCurvesTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    paramCurvesTrack->setFormationWellPath( wellPath );
    paramCurvesTrack->setFormationCase( geoMechCase );
    paramCurvesTrack->setShowRegionLabels( true );
    paramCurvesTrack->setShowWindow( false );
    std::set<RigWbsParameter> parameters = RigWbsParameter::allParameters();

    caf::ColorTable                                    colors     = RiaColorTables::contrastCategoryPaletteColors();
    std::vector<RiuQwtPlotCurveDefines::LineStyleEnum> lineStyles = { RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID,
                                                                      RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH,
                                                                      RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH_DOT };

    size_t i = 0;
    for ( const RigWbsParameter& param : parameters )
    {
        if ( !param.hasExternalSource() || param == RigWbsParameter::waterDensity() ) continue;

        RigFemResultAddress        resAddr( RIG_WELLPATH_DERIVED, param.name().toStdString(), "" );
        RimWellLogExtractionCurve* curve =
            RicWellLogTools::addWellLogExtractionCurve( paramCurvesTrack, geoMechCase, nullptr, wellPath, nullptr, -1, false, false );
        curve->setGeoMechResultAddress( resAddr );
        curve->setCurrentTimeStep( timeStep );
        curve->setColor( colors.cycledColor3f( i ) );
        curve->setLineStyle( lineStyles[i % lineStyles.size()] );
        curve->setLineThickness( 2 );
        curve->loadDataAndUpdate( false );
        curve->setCustomName( param.name() );
        i++;
    }
    paramCurvesTrack->setAutoScaleXEnabled( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createStabilityCurvesTrack( RimWellBoreStabilityPlot* plot,
                                                                     RimWellPath*              wellPath,
                                                                     RimGeoMechCase*           geoMechCase,
                                                                     int                       timeStep )
{
    RimWellLogTrack* stabilityCurvesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false,
                                                                                                  "Stability Curves",

                                                                                                  plot );
    stabilityCurvesTrack->setVisibleXRange( 0.0, 2.5 );
    stabilityCurvesTrack->setColSpan( RimPlot::THREE );
    stabilityCurvesTrack->setAutoScaleXEnabled( true );
    stabilityCurvesTrack->setTickIntervals( 1.0, 0.2 );
    stabilityCurvesTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    stabilityCurvesTrack->setFormationWellPath( wellPath );
    stabilityCurvesTrack->setFormationCase( geoMechCase );
    stabilityCurvesTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS );
    stabilityCurvesTrack->setAnnotationDisplay( RiuPlotAnnotationTool::LIGHT_LINES );
    stabilityCurvesTrack->setShowRegionLabels( false );

    std::vector<QString> resultNames = RiaResultNames::wbsDerivedResultNames();

    std::vector<cvf::Color3f> colors = { cvf::Color3f::BLUE,
                                         cvf::Color3f::BROWN,
                                         cvf::Color3f::RED,
                                         cvf::Color3f::PURPLE,
                                         cvf::Color3f::DARK_GREEN,
                                         cvf::Color3f::OLIVE };

    std::vector<RiuQwtPlotCurveDefines::LineStyleEnum> lineStyles( resultNames.size(),
                                                                   RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
    lineStyles.back() = RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH;

    for ( size_t i = 0; i < resultNames.size(); ++i )
    {
        const QString&      resultName = resultNames[i];
        RigFemResultAddress resAddr( RIG_WELLPATH_DERIVED, resultName.toStdString(), "" );
        RimWellLogWbsCurve* curve =
            RicWellLogTools::addWellLogWbsCurve( stabilityCurvesTrack, geoMechCase, nullptr, wellPath, -1, false, false );
        curve->setGeoMechResultAddress( resAddr );
        curve->setCurrentTimeStep( timeStep );
        curve->setAutoNameComponents( false, true, false, false, false );
        curve->setColor( colors[i % colors.size()] );
        curve->setLineStyle( lineStyles[i] );
        curve->setLineThickness( 2 );
        curve->loadDataAndUpdate( false );
        curve->setSmoothCurve( true );
        curve->setSmoothingThreshold( 0.002 );
        if ( resultNames[i] == RiaResultNames::wbsSHMkResult() )
        {
            curve->setCurveVisibility( false );
        }
    }

    RimWellPathCollection* wellPathCollection = nullptr;
    wellPath->firstAncestorOrThisOfTypeAsserted( wellPathCollection );

    const RimWellMeasurementCollection* measurementCollection = wellPathCollection->measurementCollection();
    for ( QString wbsMeasurementKind : RimWellMeasurement::measurementKindsForWellBoreStability() )
    {
        for ( RimWellMeasurement* measurement : measurementCollection->measurements() )
        {
            if ( measurement->wellName() == wellPath->name() && measurement->kind() == wbsMeasurementKind )
            {
                RimWellMeasurementCurve* curve =
                    RicWellLogTools::addWellMeasurementCurve( stabilityCurvesTrack, wellPath, wbsMeasurementKind );
                curve->loadDataAndUpdate( false );
                break; // Only one per measurement kind
            }
        }
    }

    stabilityCurvesTrack->setAutoScaleXEnabled( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createAnglesTrack( RimWellBoreStabilityPlot* plot,
                                                            RimWellPath*              wellPath,
                                                            RimGeoMechCase*           geoMechCase,
                                                            int                       timeStep )
{
    RimWellLogTrack* wellPathAnglesTrack =
        RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Well Path Angles", plot );
    double               minValue = 360.0, maxValue = 0.0;
    const double         angleIncrement = 90.0;
    std::vector<QString> resultNames    = RiaResultNames::wbsAngleResultNames();

    std::vector<cvf::Color3f> colors = { cvf::Color3f::GREEN, cvf::Color3f::DARK_ORANGE };

    std::vector<RiuQwtPlotCurveDefines::LineStyleEnum> lineStyles = { RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH,
                                                                      RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID };

    for ( size_t i = 0; i < resultNames.size(); ++i )
    {
        const QString&             resultName = resultNames[i];
        RigFemResultAddress        resAddr( RIG_WELLPATH_DERIVED, resultName.toStdString(), "" );
        RimWellLogExtractionCurve* curve = RicWellLogTools::addWellLogExtractionCurve( wellPathAnglesTrack,
                                                                                       geoMechCase,
                                                                                       nullptr,
                                                                                       wellPath,
                                                                                       nullptr,
                                                                                       -1,
                                                                                       false,
                                                                                       false );
        curve->setGeoMechResultAddress( resAddr );
        curve->setCurrentTimeStep( timeStep );
        curve->setCustomName( resultName );

        curve->setColor( colors[i % colors.size()] );
        curve->setLineStyle( lineStyles[i % lineStyles.size()] );
        curve->setLineThickness( 2 );

        curve->loadDataAndUpdate( false );

        double actualMinValue = minValue, actualMaxValue = maxValue;
        curve->xValueRange( &actualMinValue, &actualMaxValue );
        while ( maxValue < actualMaxValue )
        {
            maxValue += angleIncrement;
        }
        while ( minValue > actualMinValue )
        {
            minValue -= angleIncrement;
        }
        maxValue = cvf::Math::clamp( maxValue, angleIncrement, 720.0 );
        minValue = cvf::Math::clamp( minValue, 0.0, maxValue - 90.0 );
    }
    wellPathAnglesTrack->setColSpan( RimPlot::TWO );
    wellPathAnglesTrack->setVisibleXRange( minValue, maxValue );
    wellPathAnglesTrack->setTickIntervals( 180.0, 45.0 );
    wellPathAnglesTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    wellPathAnglesTrack->setFormationWellPath( wellPath );
    wellPathAnglesTrack->setFormationCase( geoMechCase );
    wellPathAnglesTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS );
    wellPathAnglesTrack->setAnnotationDisplay( RiuPlotAnnotationTool::LIGHT_LINES );
    wellPathAnglesTrack->setShowRegionLabels( false );
}
