/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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

#include "RicNewStimPlanModelPlotFeature.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RiaStimPlanModelDefines.h"
#include "RimPlotCurveAppearance.h"
#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"

#include "Well/RigWellPath.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFaciesProperties.h"
#include "RimMainPlotCollection.h"
#include "RimModeledWellPath.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCurve.h"
#include "RimStimPlanModelPlot.h"
#include "RimStimPlanModelPlotCollection.h"
#include "RimStimPlanModelTemplate.h"
#include "RimWellLogPlotNameConfig.h"
#include "RimWellLogTrack.h"
#include "RimWellPath.h"
#include "RimWellPathCollection.h"

#include "RiuPlotMainWindowTools.h"

#include "cafProgressInfo.h"
#include "cafSelectionManager.h"
#include "cvfAssert.h"
#include "cvfMath.h"

#include <QAction>
#include <QDateTime>
#include <QString>

#include <algorithm>
#include <set>

CAF_CMD_SOURCE_INIT( RicNewStimPlanModelPlotFeature, "RicNewStimPlanModelPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlot* RicNewStimPlanModelPlotFeature::createPlot( RimStimPlanModel* stimPlanModel )

{
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCaseForProperty( RiaDefines::CurveProperty::UNDEFINED );
    int             timeStep    = stimPlanModel->timeStep();

    caf::ProgressInfo progInfo( 100, "Creating StimPlan Model Plot" );

    RimStimPlanModelPlot* plot = createStimPlanModelPlot( true, "StimPlan Model" );
    plot->setStimPlanModel( stimPlanModel );

    {
        auto task = progInfo.task( "Creating formation track", 2 );
        RimEclipseCase* formationEclipseCase = stimPlanModel->eclipseCaseForType( RimExtractionConfiguration::EclipseCaseType::STATIC_CASE );
        createFormationTrack( plot, stimPlanModel, formationEclipseCase );
    }

    {
        auto            task              = progInfo.task( "Creating facies track", 2 );
        RimEclipseCase* faciesEclipseCase = stimPlanModel->eclipseCaseForProperty( RiaDefines::CurveProperty::FACIES );
        createFaciesTrack( plot, stimPlanModel, faciesEclipseCase );
    }

    {
        auto task = progInfo.task( "Creating layers track", 2 );
        createLayersTrack( plot, stimPlanModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating parameters track", 15 );

        std::vector<std::pair<QString, std::vector<RiaDefines::CurveProperty>>> plots =
            { { "Porosity", { RiaDefines::CurveProperty::POROSITY, RiaDefines::CurveProperty::POROSITY_UNSCALED } },
              { "Permeability", { RiaDefines::CurveProperty::PERMEABILITY_X, RiaDefines::CurveProperty::PERMEABILITY_Z } },
              { "Pressure", { RiaDefines::CurveProperty::INITIAL_PRESSURE, RiaDefines::CurveProperty::PRESSURE } },
              { "Net-To-Gross", { RiaDefines::CurveProperty::NET_TO_GROSS } },
              { "EQLNUM", { RiaDefines::CurveProperty::EQLNUM } } };

        std::set<QString> logarithmicPlots;
        logarithmicPlots.insert( "Permeability" );

        for ( auto result : plots )
        {
            bool logarithmicPlot = logarithmicPlots.count( result.first ) > 0;
            createParametersTrack( plot, stimPlanModel, eclipseCase, timeStep, result.first, result.second, logarithmicPlot );
        }
    }

    {
        auto task = progInfo.task( "Creating stress track", 2 );
        createParametersTrack( plot,
                               stimPlanModel,
                               eclipseCase,
                               timeStep,
                               "Stress",
                               { RiaDefines::CurveProperty::INITIAL_STRESS, RiaDefines::CurveProperty::STRESS } );
        createParametersTrack( plot, stimPlanModel, eclipseCase, timeStep, "Stress Gradient", { RiaDefines::CurveProperty::STRESS_GRADIENT } );
    }

    {
        auto task = progInfo.task( "Creating facies properties track", 15 );

        std::vector<RiaDefines::CurveProperty> results = { RiaDefines::CurveProperty::YOUNGS_MODULUS,
                                                           RiaDefines::CurveProperty::POISSONS_RATIO,
                                                           RiaDefines::CurveProperty::BIOT_COEFFICIENT,
                                                           RiaDefines::CurveProperty::K0,
                                                           RiaDefines::CurveProperty::K_IC,
                                                           RiaDefines::CurveProperty::PROPPANT_EMBEDMENT,
                                                           RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT,
                                                           RiaDefines::CurveProperty::SPURT_LOSS,
                                                           RiaDefines::CurveProperty::RELATIVE_PERMEABILITY_FACTOR,
                                                           RiaDefines::CurveProperty::PORO_ELASTIC_CONSTANT,
                                                           RiaDefines::CurveProperty::THERMAL_EXPANSION_COEFFICIENT,
                                                           RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION };

        for ( auto result : results )
        {
            QString trackName = caf::AppEnum<RiaDefines::CurveProperty>::uiText( result );
            createParametersTrack( plot, stimPlanModel, eclipseCase, timeStep, trackName, { result } );
        }
    }

    {
        auto task = progInfo.task( "Creating temperature track", 2 );
        createParametersTrack( plot, stimPlanModel, eclipseCase, timeStep, "Temperature", { RiaDefines::CurveProperty::TEMPERATURE } );
    }

    {
        auto task = progInfo.task( "Updating all tracks", 5 );

        plot->setPlotTitleVisible( true );
        plot->setLegendsVisible( true );
        plot->setLegendsHorizontal( false );
        plot->setDepthType( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH );
        plot->setAutoScaleDepthValuesEnabled( true );
    }

    RiuPlotMainWindowTools::showPlotMainWindow();
    RiuPlotMainWindowTools::onObjectAppended( plot );

    plot->loadDataAndUpdate();

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelPlotFeature::onActionTriggered( bool isChecked )
{
    RimStimPlanModel* stimPlanModel = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimStimPlanModel>();
    if ( !stimPlanModel ) return;

    createPlot( stimPlanModel );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New StimPlan Model Plot" );
    // actionToSetup->setIcon( QIcon( ":/WellBoreStability16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelPlotFeature::createFormationTrack( RimStimPlanModelPlot* plot, RimStimPlanModel* stimPlanModel, RimEclipseCase* eclipseCase )
{
    RimWellLogTrack* formationTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Formations", plot );
    formationTrack->setFormationWellPath( stimPlanModel->thicknessDirectionWellPath() );
    formationTrack->setFormationCase( eclipseCase );
    formationTrack->setAnnotationType( RiaDefines::RegionAnnotationType::FORMATION_ANNOTATIONS );
    formationTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AxisGridVisibility::AXIS_GRID_NONE );
    formationTrack->setShowWellPathAttributes( true );
    formationTrack->setShowBothSidesOfWell( false );
    formationTrack->setWellPathAttributesSource( stimPlanModel->thicknessDirectionWellPath() );
    formationTrack->setVisiblePropertyValueRange( 0.0, 0.0 );
    formationTrack->setOverburdenHeight( stimPlanModel->overburdenHeight() );
    formationTrack->setUnderburdenHeight( stimPlanModel->underburdenHeight() );
    formationTrack->setColSpan( RimPlot::ONE );
    formationTrack->setPropertyValueAxisTitle( stimPlanModel->unitForProperty( RiaDefines::CurveProperty::FORMATIONS ) );
    formationTrack->setLegendsVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelPlotFeature::createFaciesTrack( RimStimPlanModelPlot* plot, RimStimPlanModel* stimPlanModel, RimEclipseCase* eclipseCase )
{
    RimStimPlanModelTemplate* stimPlanModelTemplate = stimPlanModel->stimPlanModelTemplate();
    if ( !stimPlanModelTemplate ) return;

    RimFaciesProperties* faciesProperties = stimPlanModelTemplate->faciesProperties();
    if ( !faciesProperties ) return;

    const RimEclipseResultDefinition* faciesDefinition = faciesProperties->faciesDefinition();
    if ( !faciesDefinition ) return;

    RimWellLogTrack* faciesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Facies", plot );
    faciesTrack->setFormationWellPath( stimPlanModel->thicknessDirectionWellPath() );
    faciesTrack->setFormationCase( eclipseCase );
    faciesTrack->setAnnotationType( RiaDefines::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS );
    faciesTrack->setRegionPropertyResultType( faciesDefinition->resultType(), faciesDefinition->resultVariable() );
    faciesTrack->setAnnotationDisplay( RiaDefines::COLOR_SHADING );
    faciesTrack->setOverburdenHeight( stimPlanModel->overburdenHeight() );
    faciesTrack->setUnderburdenHeight( stimPlanModel->underburdenHeight() );
    faciesTrack->setPropertyValueAxisTitle( stimPlanModel->unitForProperty( RiaDefines::CurveProperty::FACIES ) );
    faciesTrack->setLegendsVisible( false );
    faciesTrack->setPlotTitleVisible( true );

    RimColorLegend* faciesColors = faciesProperties->colorLegend();
    if ( faciesColors ) faciesTrack->setColorShadingLegend( faciesColors );

    faciesTrack->setVisiblePropertyValueRange( 0.0, 0.0 );
    faciesTrack->setColSpan( RimPlot::ONE );
    faciesTrack->setAutoScalePropertyValuesEnabled( false );
    faciesTrack->setVisiblePropertyValueRange( 0.0, 0.0 );
    faciesTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AxisGridVisibility::AXIS_GRID_NONE );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimStimPlanModelCurve* curve = new RimStimPlanModelCurve;
    curve->setCurveProperty( RiaDefines::CurveProperty::FACIES );
    curve->setStimPlanModel( stimPlanModel );
    curve->setEclipseResultCategory( faciesDefinition->resultType() );
    curve->setEclipseResultVariable( faciesDefinition->resultVariable() );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_NONE );
    curve->setLineThickness( 2 );
    curve->setAutoNameComponents( false, true, false, false, false );

    faciesTrack->addCurve( curve );
    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();
    faciesTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelPlotFeature::createLayersTrack( RimStimPlanModelPlot* plot, RimStimPlanModel* stimPlanModel, RimEclipseCase* eclipseCase )
{
    RimWellLogTrack* faciesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Layers", plot );
    faciesTrack->setFormationWellPath( stimPlanModel->thicknessDirectionWellPath() );
    faciesTrack->setFormationCase( eclipseCase );
    faciesTrack->setLegendsVisible( true );
    faciesTrack->setPlotTitleVisible( true );

    RimStimPlanModelTemplate* stimPlanModelTemplate = stimPlanModel->stimPlanModelTemplate();
    if ( !stimPlanModelTemplate ) return;

    RimFaciesProperties* faciesProperties = stimPlanModelTemplate->faciesProperties();
    if ( !faciesProperties ) return;

    RimColorLegend* faciesColors = faciesProperties->colorLegend();
    if ( faciesColors ) faciesTrack->setColorShadingLegend( faciesColors );

    faciesTrack->setVisiblePropertyValueRange( 0.0, 0.0 );
    faciesTrack->setColSpan( RimPlot::ONE );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimStimPlanModelCurve* curve = new RimStimPlanModelCurve;
    curve->setCurveProperty( RiaDefines::CurveProperty::LAYERS );
    curve->setStimPlanModel( stimPlanModel );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID );
    curve->setLineThickness( 2 );
    curve->setAutoNameComponents( false, true, false, false, false );

    faciesTrack->addCurve( curve );
    faciesTrack->setAutoScalePropertyValuesEnabled( true );
    faciesTrack->setPropertyValueAxisTitle( stimPlanModel->unitForProperty( RiaDefines::CurveProperty::LAYERS ) );

    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();
    faciesTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelPlotFeature::createParametersTrack( RimStimPlanModelPlot*                         plot,
                                                            RimStimPlanModel*                             stimPlanModel,
                                                            RimEclipseCase*                               eclipseCase,
                                                            int                                           timeStep,
                                                            const QString&                                trackTitle,
                                                            const std::vector<RiaDefines::CurveProperty>& propertyTypes,
                                                            bool                                          isPlotLogarithmic )
{
    CAF_ASSERT( !propertyTypes.empty() );

    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, trackTitle, plot );
    plotTrack->setFormationCase( eclipseCase );
    plotTrack->setFormationWellPath( stimPlanModel->thicknessDirectionWellPath() );
    plotTrack->setColSpan( defaultColSpan( propertyTypes[0] ) );
    plotTrack->setLegendsVisible( true );
    plotTrack->setPlotTitleVisible( true );
    plotTrack->setShowWindow( shouldShowByDefault( propertyTypes, stimPlanModel->useDetailedFluidLoss() ) );

    int colorIndex = 0;
    for ( const RiaDefines::CurveProperty& propertyType : propertyTypes )
    {
        QString                   resultVariable     = stimPlanModel->eclipseResultVariable( propertyType );
        RiaDefines::ResultCatType resultCategoryType = stimPlanModel->eclipseResultCategory( propertyType );
        // TODO: maybe improve?
        bool fixedInitialTimeStep = ( propertyType == RiaDefines::CurveProperty::INITIAL_PRESSURE ||
                                      resultCategoryType == RiaDefines::ResultCatType::STATIC_NATIVE );

        RimStimPlanModelCurve* curve = new RimStimPlanModelCurve;
        curve->setCurveProperty( propertyType );
        curve->setStimPlanModel( stimPlanModel );
        curve->setEclipseResultVariable( resultVariable );
        curve->setEclipseResultCategory( resultCategoryType );
        curve->setColor( defaultColor( propertyType, colorIndex ) );

        RimPlotCurveAppearance::FillStyle fillStyle = defaultFillStyle( propertyType );
        if ( fillStyle != Qt::NoBrush )
        {
            curve->setFillStyle( fillStyle );
            curve->setFillColor( defaultFillColor( propertyType ) );
        }

        curve->setLineStyle( defaultLineStyle( propertyType ) );
        curve->setLineThickness( 2 );

        if ( propertyType == RiaDefines::CurveProperty::STRESS_GRADIENT )
        {
            curve->setInterpolation( RiuQwtPlotCurveDefines::CurveInterpolationEnum::INTERPOLATION_STEP_LEFT );
        }

        if ( fixedInitialTimeStep )
        {
            curve->setAutoNameComponents( false, false, false, false, false );
            curve->setCurrentTimeStep( 0 );
        }
        else
        {
            curve->setAutoNameComponents( false, true, false, false, false );
            curve->setCurrentTimeStep( timeStep );
        }

        plotTrack->addCurve( curve );
        curve->loadDataAndUpdate( true );

        curve->updateConnectedEditors();

        colorIndex++;
    }

    plotTrack->setPropertyValueAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setLogarithmicScale( isPlotLogarithmic );
    plotTrack->setAutoScalePropertyValuesEnabled( true );
    plotTrack->setMinAndMaxTicksOnly( useMinMaxTicksOnly( propertyTypes[0] ) );
    plotTrack->setPropertyValueAxisTitle( stimPlanModel->unitForProperty( propertyTypes[0] ) );

    plotTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlot* RicNewStimPlanModelPlotFeature::createStimPlanModelPlot( bool showAfterCreation, const QString& plotDescription )

{
    RimStimPlanModelPlotCollection* stimPlanModelPlotColl = stimPlanModelPlotCollection();
    CVF_ASSERT( stimPlanModelPlotColl );

    // Make sure the summary plot window is created
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();

    RimStimPlanModelPlot* plot = new RimStimPlanModelPlot();
    plot->setAsPlotMdiWindow();

    stimPlanModelPlotColl->addStimPlanModelPlot( plot );

    plot->setNamingMethod( RiaDefines::ObjectNamingMethod::CUSTOM );
    if ( !plotDescription.isEmpty() )
    {
        plot->nameConfig()->setCustomName( plotDescription );
    }
    else
    {
        plot->nameConfig()->setCustomName(
            QString( "StimPlan Model Plot %1" ).arg( stimPlanModelPlotCollection()->stimPlanModelPlots().size() ) );
    }

    stimPlanModelPlotColl->updateAllRequiredEditors();

    if ( showAfterCreation )
    {
        RiaGuiApplication::instance()->getOrCreateAndShowMainPlotWindow();
    }

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimStimPlanModelPlotCollection* RicNewStimPlanModelPlotFeature::stimPlanModelPlotCollection()
{
    return RimMainPlotCollection::current()->stimPlanModelPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStimPlanModelPlotFeature::shouldShowByDefault( const std::vector<RiaDefines::CurveProperty>& propertyTypes, bool useDetailedFluidLoss )
{
    std::vector<RiaDefines::CurveProperty> defaultPropertyTypes = {
        RiaDefines::CurveProperty::FACIES,
        RiaDefines::CurveProperty::POROSITY,
        RiaDefines::CurveProperty::INITIAL_PRESSURE,
        RiaDefines::CurveProperty::PRESSURE,
        RiaDefines::CurveProperty::PERMEABILITY_X,
        RiaDefines::CurveProperty::PERMEABILITY_Z,
        RiaDefines::CurveProperty::STRESS,
        RiaDefines::CurveProperty::INITIAL_STRESS,
        RiaDefines::CurveProperty::STRESS_GRADIENT,
        RiaDefines::CurveProperty::YOUNGS_MODULUS,
        RiaDefines::CurveProperty::POISSONS_RATIO,
        RiaDefines::CurveProperty::K_IC,
        RiaDefines::CurveProperty::PROPPANT_EMBEDMENT,
        RiaDefines::CurveProperty::BIOT_COEFFICIENT,
        RiaDefines::CurveProperty::K0,
    };

    if ( !useDetailedFluidLoss )
    {
        defaultPropertyTypes.push_back( RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT );
    }

    for ( auto propertyType : propertyTypes )
    {
        for ( auto defaultPropertyType : defaultPropertyTypes )
        {
            if ( propertyType == defaultPropertyType ) return true;
        }
    }
    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RicNewStimPlanModelPlotFeature::defaultColor( RiaDefines::CurveProperty property, int colorIndex )
{
    std::map<RiaDefines::CurveProperty, cvf::Color3f> colorMap;

    colorMap[RiaDefines::CurveProperty::LAYERS]             = cvf::Color3f( 0.000f, 0.000f, 1.000f );
    colorMap[RiaDefines::CurveProperty::POROSITY]           = cvf::Color3f( 0.804f, 0.522f, 0.247f );
    colorMap[RiaDefines::CurveProperty::POROSITY_UNSCALED]  = cvf::Color3f::BLACK;
    colorMap[RiaDefines::CurveProperty::PERMEABILITY_X]     = cvf::Color3f( 0.745f, 0.000f, 0.000f );
    colorMap[RiaDefines::CurveProperty::PERMEABILITY_Z]     = cvf::Color3f::RED;
    colorMap[RiaDefines::CurveProperty::INITIAL_PRESSURE]   = cvf::Color3f::BLACK;
    colorMap[RiaDefines::CurveProperty::PRESSURE]           = cvf::Color3f( 0.000f, 0.333f, 1.000f );
    colorMap[RiaDefines::CurveProperty::INITIAL_STRESS]     = cvf::Color3f::BLACK;
    colorMap[RiaDefines::CurveProperty::STRESS]             = cvf::Color3f( 0.541f, 0.169f, 0.886f );
    colorMap[RiaDefines::CurveProperty::STRESS_GRADIENT]    = cvf::Color3f( 0.522f, 0.522f, 0.784f );
    colorMap[RiaDefines::CurveProperty::YOUNGS_MODULUS]     = cvf::Color3f( 0.565f, 0.565f, 0.424f );
    colorMap[RiaDefines::CurveProperty::POISSONS_RATIO]     = cvf::Color3f( 0.804f, 0.522f, 0.247f );
    colorMap[RiaDefines::CurveProperty::BIOT_COEFFICIENT]   = cvf::Color3f( 0.000f, 0.506f, 0.761f );
    colorMap[RiaDefines::CurveProperty::K0]                 = cvf::Color3f( 0.294f, 0.765f, 0.529f );
    colorMap[RiaDefines::CurveProperty::K_IC]               = cvf::Color3f( 0.576f, 0.576f, 0.000f );
    colorMap[RiaDefines::CurveProperty::PROPPANT_EMBEDMENT] = cvf::Color3f( 0.667f, 0.333f, 0.498f );

    if ( colorMap.count( property ) > 0 ) return colorMap[property];

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();
    return colors.cycledColor3f( colorIndex );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
cvf::Color3f RicNewStimPlanModelPlotFeature::defaultFillColor( RiaDefines::CurveProperty property )
{
    std::map<RiaDefines::CurveProperty, cvf::Color3f> colorMap;

    colorMap[RiaDefines::CurveProperty::POROSITY]           = cvf::Color3f( 0.988f, 0.635f, 0.302f );
    colorMap[RiaDefines::CurveProperty::PERMEABILITY_X]     = cvf::Color3f( 1.000f, 0.486f, 0.486f );
    colorMap[RiaDefines::CurveProperty::PERMEABILITY_Z]     = cvf::Color3f( 1.000f, 0.486f, 0.486f );
    colorMap[RiaDefines::CurveProperty::STRESS_GRADIENT]    = cvf::Color3f( 0.667f, 0.667f, 1.000f );
    colorMap[RiaDefines::CurveProperty::YOUNGS_MODULUS]     = cvf::Color3f( 0.667f, 0.667f, 0.498f );
    colorMap[RiaDefines::CurveProperty::POISSONS_RATIO]     = cvf::Color3f( 1.000f, 0.667f, 0.498f );
    colorMap[RiaDefines::CurveProperty::BIOT_COEFFICIENT]   = cvf::Color3f( 0.647f, 0.847f, 1.000f );
    colorMap[RiaDefines::CurveProperty::K0]                 = cvf::Color3f( 0.482f, 0.765f, 0.573f );
    colorMap[RiaDefines::CurveProperty::K_IC]               = cvf::Color3f( 0.839f, 0.729f, 0.941f );
    colorMap[RiaDefines::CurveProperty::PROPPANT_EMBEDMENT] = cvf::Color3f( 0.882f, 0.439f, 0.663f );

    if ( colorMap.count( property ) > 0 ) return colorMap[property];

    return cvf::Color3f::LIGHT_GRAY;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlotCurveAppearance::FillStyle RicNewStimPlanModelPlotFeature::defaultFillStyle( RiaDefines::CurveProperty property )
{
    std::set<RiaDefines::CurveProperty> solids = {
        RiaDefines::CurveProperty::POROSITY,
        RiaDefines::CurveProperty::PERMEABILITY_X,
        RiaDefines::CurveProperty::PERMEABILITY_Z,
        RiaDefines::CurveProperty::STRESS_GRADIENT,
        RiaDefines::CurveProperty::YOUNGS_MODULUS,
        RiaDefines::CurveProperty::POISSONS_RATIO,
        RiaDefines::CurveProperty::BIOT_COEFFICIENT,
        RiaDefines::CurveProperty::K0,
        RiaDefines::CurveProperty::K_IC,
        RiaDefines::CurveProperty::PROPPANT_EMBEDMENT,
    };

    if ( solids.count( property ) > 0 ) return RimPlotCurveAppearance::FillStyle( Qt::SolidPattern );

    return RimPlotCurveAppearance::FillStyle( Qt::NoBrush );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiuQwtPlotCurveDefines::LineStyleEnum RicNewStimPlanModelPlotFeature::defaultLineStyle( RiaDefines::CurveProperty property )
{
    std::set<RiaDefines::CurveProperty> dashedProperties = { RiaDefines::CurveProperty::INITIAL_STRESS,
                                                             RiaDefines::CurveProperty::INITIAL_PRESSURE };

    if ( dashedProperties.count( property ) > 0 ) return RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_DASH;

    return RiuQwtPlotCurveDefines::LineStyleEnum::STYLE_SOLID;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimPlot::RowOrColSpan RicNewStimPlanModelPlotFeature::defaultColSpan( RiaDefines::CurveProperty property )
{
    std::set<RiaDefines::CurveProperty> wideProperties = { RiaDefines::CurveProperty::STRESS,
                                                           RiaDefines::CurveProperty::INITIAL_STRESS,
                                                           RiaDefines::CurveProperty::PRESSURE,
                                                           RiaDefines::CurveProperty::INITIAL_PRESSURE };

    if ( wideProperties.count( property ) > 0 ) return RimPlot::TWO;

    return RimPlot::ONE;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStimPlanModelPlotFeature::useMinMaxTicksOnly( RiaDefines::CurveProperty property )
{
    std::set<RiaDefines::CurveProperty> useMajorAndMinorTickmarks = { RiaDefines::CurveProperty::STRESS,
                                                                      RiaDefines::CurveProperty::INITIAL_STRESS,
                                                                      RiaDefines::CurveProperty::PRESSURE,
                                                                      RiaDefines::CurveProperty::INITIAL_PRESSURE };

    return useMajorAndMinorTickmarks.count( property ) == 0;
}
