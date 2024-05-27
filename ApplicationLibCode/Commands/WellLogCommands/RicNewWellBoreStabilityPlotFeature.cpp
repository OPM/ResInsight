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
#include "RiaPlotDefines.h"
#include "RiaResultNames.h"

#include "RicNewWellLogPlotFeatureImpl.h"
#include "RicWellLogTools.h"

#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "RigWellPath.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"
#include "RimProject.h"
#include "RimTools.h"
#include "RimWellBoreStabilityPlot.h"
#include "RimWellLogChannel.h"
#include "RimWellLogExtractionCurve.h"
#include "RimWellLogLasFile.h"
#include "RimWellLogLasFileCurve.h"
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
#include "cvfColor3.h"
#include "cvfMath.h"

#include <QAction>
#include <QDateTime>
#include <QString>

#include <algorithm>
#include <set>

CAF_CMD_SOURCE_INIT( RicNewWellBoreStabilityPlotFeature, "RicNewWellBoreStabilityPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimWellBoreStabilityPlot* RicNewWellBoreStabilityPlotFeature::createPlot( RimGeoMechCase*         geoMechCase,
                                                                          RimWellPath*            wellPath,
                                                                          int                     timeStep,
                                                                          const RimWbsParameters* parameters /* = nullptr*/ )
{
    caf::ProgressInfo progInfo( 100, "Creating Well Bore Stability Plot" );

    RimWellBoreStabilityPlot* plot = RicNewWellLogPlotFeatureImpl::createWellBoreStabilityPlot( false, "Well Bore Stability", parameters );
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

        QString templateText = QString( "Well Bore Stability: %1, %2, %3, %4, %5" )
                                   .arg( RiaDefines::namingVariableCase() )
                                   .arg( RiaDefines::namingVariableWell() )
                                   .arg( RiaDefines::namingVariableTime() )
                                   .arg( RiaDefines::namingVariableAirGap() )
                                   .arg( RiaDefines::namingVariableWaterDepth() );
        plot->setNameTemplateText( templateText );
        plot->setNamingMethod( RiaDefines::ObjectNamingMethod::TEMPLATE );

        plot->setPlotTitleVisible( true );
        plot->setLegendsVisible( true );
        plot->setLegendsHorizontal( true );
        plot->setDepthType( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH_RKB );
        plot->setAutoScaleDepthValuesEnabled( true );

        RicNewWellLogPlotFeatureImpl::updateAfterCreation( plot );
    }

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::onObjectAppended( plot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewWellBoreStabilityPlotFeature::isCommandEnabled() const
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
    RimWellPath*              wellPath       = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimWellPath>();
    RimWellLogPlotCollection* plotCollection = caf::SelectionManager::instance()->selectedItemOfType<RimWellLogPlotCollection>();
    if ( !wellPath )
    {
        if ( plotCollection )
        {
            RimProject*               project      = RimProject::current();
            std::vector<RimWellPath*> allWellPaths = project->allWellPaths();
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
        RiaLogging::error( QString( "The well path %1 has no geometry. Cannot create a Well Bore Stability Plot" ).arg( wellPath->name() ) );
        return;
    }
    if ( wellPathGeometry->rkbDiff() == HUGE_VAL )
    {
        RiaLogging::error( QString( "The well path %1 has no datum elevation and we cannot estimate TVDRKB. Cannot "
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
void RicNewWellBoreStabilityPlotFeature::createFormationTrack( RimWellBoreStabilityPlot* plot, RimWellPath* wellPath, RimGeoMechCase* geoMechCase )
{
    RimWellLogTrack* formationTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Formations", plot );
    formationTrack->setFormationWellPath( wellPath );
    formationTrack->setFormationCase( geoMechCase );
    formationTrack->setAnnotationType( RiaDefines::RegionAnnotationType::FORMATION_ANNOTATIONS );
    formationTrack->setVisiblePropertyValueRange( 0.0, 0.0 );
    formationTrack->enablePropertyAxis( false );
    formationTrack->setColSpan( RimPlot::ONE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createCasingShoeTrack( RimWellBoreStabilityPlot* plot, RimWellPath* wellPath, RimGeoMechCase* geoMechCase )
{
    RimWellLogTrack* casingShoeTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Well Design", plot );
    casingShoeTrack->setColSpan( RimPlot::ONE );
    casingShoeTrack->setFormationWellPath( wellPath );
    casingShoeTrack->setFormationCase( geoMechCase );
    casingShoeTrack->setAnnotationType( RiaDefines::RegionAnnotationType::FORMATION_ANNOTATIONS );
    casingShoeTrack->setAnnotationDisplay( RiaDefines::COLOR_SHADING_AND_LINES );
    casingShoeTrack->setShowRegionLabels( false );
    casingShoeTrack->setShowWellPathAttributes( true );
    casingShoeTrack->setShowBothSidesOfWell( false );
    casingShoeTrack->setAnnotationTransparency( 90 );
    casingShoeTrack->setWellPathAttributesSource( wellPath );
    casingShoeTrack->setVisiblePropertyValueRange( 0.0, 0.0 );
    casingShoeTrack->enablePropertyAxis( false );
    casingShoeTrack->setAutoScalePropertyValuesEnabled( true );
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
    RimWellLogTrack* paramCurvesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "WBS Parameters", plot );
    paramCurvesTrack->setColSpan( RimPlot::TWO );
    paramCurvesTrack->setVisiblePropertyValueRange( 0.0, 2.0 );
    paramCurvesTrack->setAutoScalePropertyValuesEnabled( true );
    paramCurvesTrack->setTickIntervals( 1.0, 0.2 );
    paramCurvesTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    paramCurvesTrack->setFormationWellPath( wellPath );
    paramCurvesTrack->setFormationCase( geoMechCase );
    paramCurvesTrack->setShowRegionLabels( true );
    paramCurvesTrack->setShowWindow( false );
    std::set<RigWbsParameter> parameters = parametersForTrack();

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
        curve->setAutoNameComponents( false, true, false, false, false );
        curve->updateCurveName();

        i++;
    }
    paramCurvesTrack->setAutoScalePropertyValuesEnabled( true );
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
    stabilityCurvesTrack->setVisiblePropertyValueRange( 0.0, 2.5 );
    stabilityCurvesTrack->setColSpan( RimPlot::THREE );
    stabilityCurvesTrack->setAutoScalePropertyValuesEnabled( true );
    stabilityCurvesTrack->setTickIntervals( 1.0, 0.2 );
    stabilityCurvesTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    stabilityCurvesTrack->setFormationWellPath( wellPath );
    stabilityCurvesTrack->setFormationCase( geoMechCase );
    stabilityCurvesTrack->setAnnotationType( RiaDefines::RegionAnnotationType::FORMATION_ANNOTATIONS );
    stabilityCurvesTrack->setAnnotationDisplay( RiaDefines::LIGHT_LINES );
    stabilityCurvesTrack->setShowRegionLabels( false );

    std::vector<QString> resultNames = RiaResultNames::wbsDerivedResultNames();

    std::vector<cvf::Color3f> colors =
        { cvf::Color3f::BLUE, cvf::Color3f::BROWN, cvf::Color3f::RED, cvf::Color3f::PURPLE, cvf::Color3f::DARK_GREEN, cvf::Color3f::OLIVE };

    std::vector<RiuQwtPlotCurveDefines::LineStyleEnum> lineStyles( resultNames.size(), RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
    lineStyles.back() = RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH;

    std::set<QString> defaultOffResults = {
        RiaResultNames::wbsSHMkResult(),
        RiaResultNames::wbsSHMkExpResult(),
        RiaResultNames::wbsSHMkMinResult(),
        RiaResultNames::wbsSHMkMaxResult(),
        RiaResultNames::wbsFGMkExpResult(),
        RiaResultNames::wbsFGMkMinResult(),
    };

    for ( size_t i = 0; i < resultNames.size(); ++i )
    {
        const QString&      resultName = resultNames[i];
        RigFemResultAddress resAddr( RIG_WELLPATH_DERIVED, resultName.toStdString(), "" );
        RimWellLogWbsCurve* curve =
            RicWellLogTools::addWellLogWbsCurve( stabilityCurvesTrack, geoMechCase, nullptr, wellPath, -1, false, false );
        curve->setGeoMechResultAddress( resAddr );
        curve->setCurrentTimeStep( timeStep );
        curve->setAutoNameComponents( false, true, false, false, false );
        auto [color, lineStyle] = getColorAndLineStyle( resultName, i, colors );
        curve->setColor( color );
        curve->setLineStyle( lineStyle );
        curve->setLineThickness( 2 );
        curve->loadDataAndUpdate( false );
        curve->setSmoothCurve( true );
        curve->setSmoothingThreshold( 0.002 );
        if ( defaultOffResults.count( resultNames[i] ) )
        {
            curve->setCheckState( false );
        }
        curve->updateCurveName();
    }

    RimWellPathCollection* wellPathCollection = RimTools::wellPathCollection();

    const RimWellMeasurementCollection* measurementCollection = wellPathCollection->measurementCollection();
    for ( QString wbsMeasurementKind : RimWellMeasurement::measurementKindsForWellBoreStability() )
    {
        for ( RimWellMeasurement* measurement : measurementCollection->measurements() )
        {
            if ( measurement->wellName() == wellPath->name() && measurement->kind() == wbsMeasurementKind )
            {
                RimWellMeasurementCurve* curve = RicWellLogTools::addWellMeasurementCurve( stabilityCurvesTrack, wellPath, wbsMeasurementKind );
                curve->loadDataAndUpdate( false );
                break; // Only one per measurement kind
            }
        }
    }

    stabilityCurvesTrack->setAutoScalePropertyValuesEnabled( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::pair<cvf::Color3f, RiuQwtPlotCurveDefines::LineStyleEnum>
    RicNewWellBoreStabilityPlotFeature::getColorAndLineStyle( const QString& resultName, size_t i, const std::vector<cvf::Color3f>& colors )
{
    if ( resultName == RiaResultNames::wbsSHMkResult() ) return { cvf::Color3f::GREEN, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID };
    if ( resultName == RiaResultNames::wbsSHMkExpResult() )
        return { cvf::Color3f::GREEN, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH };
    if ( resultName == RiaResultNames::wbsSHMkMinResult() )
        return { cvf::Color3f::GREEN, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DOT };
    if ( resultName == RiaResultNames::wbsSHMkMaxResult() )
        return { cvf::Color3f::GREEN, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH_DOT };

    if ( resultName == RiaResultNames::wbsFGMkExpResult() )
        return { cvf::Color3f::BLUE, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH };
    if ( resultName == RiaResultNames::wbsFGMkMinResult() ) return { cvf::Color3f::BLUE, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DOT };

    if ( resultName == RiaResultNames::wbsPPInitialResult() )
        return { cvf::Color3f::DEEP_PINK, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID };
    if ( resultName == RiaResultNames::wbsPPExpResult() )
        return { cvf::Color3f::DEEP_PINK, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH };
    if ( resultName == RiaResultNames::wbsPPMinResult() )
        return { cvf::Color3f::DEEP_PINK, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DOT };
    if ( resultName == RiaResultNames::wbsPPMaxResult() )
        return { cvf::Color3f::DEEP_PINK, RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH_DOT };

    return { colors[i % colors.size()], RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewWellBoreStabilityPlotFeature::createAnglesTrack( RimWellBoreStabilityPlot* plot,
                                                            RimWellPath*              wellPath,
                                                            RimGeoMechCase*           geoMechCase,
                                                            int                       timeStep )
{
    RimWellLogTrack*     wellPathAnglesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Well Path Angles", plot );
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
        RimWellLogExtractionCurve* curve =
            RicWellLogTools::addWellLogExtractionCurve( wellPathAnglesTrack, geoMechCase, nullptr, wellPath, nullptr, -1, false, false );
        curve->setGeoMechResultAddress( resAddr );
        curve->setCurrentTimeStep( timeStep );
        curve->setCustomName( resultName );

        curve->setColor( colors[i % colors.size()] );
        curve->setLineStyle( lineStyles[i % lineStyles.size()] );
        curve->setLineThickness( 2 );

        curve->loadDataAndUpdate( false );

        maxValue = cvf::Math::clamp( maxValue, angleIncrement, 720.0 );
        minValue = cvf::Math::clamp( minValue, 0.0, maxValue - 90.0 );
    }
    wellPathAnglesTrack->setColSpan( RimPlot::TWO );
    wellPathAnglesTrack->setVisiblePropertyValueRange( minValue, maxValue );
    wellPathAnglesTrack->setTickIntervals( 180.0, 45.0 );
    wellPathAnglesTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    wellPathAnglesTrack->setFormationWellPath( wellPath );
    wellPathAnglesTrack->setFormationCase( geoMechCase );
    wellPathAnglesTrack->setAnnotationType( RiaDefines::RegionAnnotationType::FORMATION_ANNOTATIONS );
    wellPathAnglesTrack->setAnnotationDisplay( RiaDefines::LIGHT_LINES );
    wellPathAnglesTrack->setShowRegionLabels( false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RigWbsParameter> RicNewWellBoreStabilityPlotFeature::parametersForTrack()
{
    return { RigWbsParameter::PP_Reservoir(),
             RigWbsParameter::PP_NonReservoir(),
             RigWbsParameter::poissonRatio(),
             RigWbsParameter::UCS(),
             RigWbsParameter::OBG(),
             RigWbsParameter::OBG0(),
             RigWbsParameter::SH(),
             RigWbsParameter::DF(),
             RigWbsParameter::K0_FG(),
             RigWbsParameter::K0_SH(),
             RigWbsParameter::FG_Shale(),
             RigWbsParameter::waterDensity() };
}
