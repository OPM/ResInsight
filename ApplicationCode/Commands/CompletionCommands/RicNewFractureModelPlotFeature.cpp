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

#include "RicNewFractureModelPlotFeature.h"

#include "RiaApplication.h"
#include "RiaColorTables.h"
#include "RiaFractureDefines.h"
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"

#include "RigWellPath.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimElasticPropertiesCurve.h"
#include "RimFractureModel.h"
#include "RimFractureModelCurve.h"
#include "RimFractureModelPlot.h"
#include "RimFractureModelPlotCollection.h"
#include "RimLayerCurve.h"
#include "RimMainPlotCollection.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
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

CAF_CMD_SOURCE_INIT( RicNewFractureModelPlotFeature, "RicNewFractureModelPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlot*
    RicNewFractureModelPlotFeature::createPlot( RimEclipseCase* eclipseCase, RimFractureModel* fractureModel, int timeStep )

{
    caf::ProgressInfo progInfo( 100, "Creating Fracture Model Plot" );

    RimFractureModelPlot* plot = createFractureModelPlot( true, "Fracture Model" );
    plot->setFractureModel( fractureModel );

    {
        auto task = progInfo.task( "Creating formation track", 2 );
        createFormationTrack( plot, fractureModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating facies track", 2 );
        createFaciesTrack( plot, fractureModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating layers track", 2 );
        createLayersTrack( plot, fractureModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating parameters track", 15 );

        std::map<QString, PlotDefVector> plots;
        plots["Porosity"] = {std::make_tuple( "PORO",
                                              RiaDefines::ResultCatType::STATIC_NATIVE,
                                              RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE,
                                              false,
                                              RiaDefines::CurveProperty::POROSITY )};

        plots["Pressure"] = {std::make_tuple( "PRESSURE",
                                              RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                              RimFractureModelCurve::MissingValueStrategy::LINEAR_INTERPOLATION,
                                              true,
                                              RiaDefines::CurveProperty::INITIAL_PRESSURE ),
                             std::make_tuple( "PRESSURE",
                                              RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                              RimFractureModelCurve::MissingValueStrategy::OTHER_CURVE_PROPERTY,
                                              false,
                                              RiaDefines::CurveProperty::PRESSURE )};

        plots["Permeability"] = {std::make_tuple( "PERMX",
                                                  RiaDefines::ResultCatType::STATIC_NATIVE,
                                                  RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE,
                                                  false,
                                                  RiaDefines::CurveProperty::PERMEABILITY_X ),
                                 std::make_tuple( "PERMZ",
                                                  RiaDefines::ResultCatType::STATIC_NATIVE,
                                                  RimFractureModelCurve::MissingValueStrategy::DEFAULT_VALUE,
                                                  false,
                                                  RiaDefines::CurveProperty::PERMEABILITY_Z )};

        std::set<QString> logarithmicPlots;
        logarithmicPlots.insert( "Permeability" );

        for ( auto result : plots )
        {
            bool logarithmicPlot = logarithmicPlots.count( result.first ) > 0;
            createParametersTrack( plot, fractureModel, eclipseCase, timeStep, result.first, result.second, logarithmicPlot );
        }
    }

    {
        auto task = progInfo.task( "Creating stress track", 2 );
        createStressTrack( plot, fractureModel, eclipseCase, timeStep, RiaDefines::CurveProperty::STRESS );
        createStressTrack( plot, fractureModel, eclipseCase, timeStep, RiaDefines::CurveProperty::STRESS_GRADIENT );
    }

    {
        auto task = progInfo.task( "Creating facies properties track", 15 );

        std::vector<RiaDefines::CurveProperty> results = {RiaDefines::CurveProperty::YOUNGS_MODULUS,
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
                                                          RiaDefines::CurveProperty::IMMOBILE_FLUID_SATURATION};

        for ( auto result : results )
        {
            createElasticPropertiesTrack( plot, fractureModel, eclipseCase, timeStep, result );
        }
    }

    {
        auto task = progInfo.task( "Creating temperature track", 2 );
        createStressTrack( plot, fractureModel, eclipseCase, timeStep, RiaDefines::CurveProperty::TEMPERATURE );
    }

    {
        auto task = progInfo.task( "Updating all tracks", 5 );

        plot->nameConfig()->setAutoNameTags( false, false, false, false, false );
        plot->setPlotTitleVisible( true );
        plot->setLegendsVisible( true );
        plot->setLegendsHorizontal( true );
        plot->setDepthType( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH );
        plot->setAutoScaleDepthEnabled( true );
    }

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );

    // Make sure the summary plot window is visible
    RiuPlotMainWindowTools::showPlotMainWindow();

    plot->loadDataAndUpdate();

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewFractureModelPlotFeature::isCommandEnabled()
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view ) return false;
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( view );
    return eclipseView != nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::onActionTriggered( bool isChecked )
{
    RimFractureModel* fractureModel = caf::SelectionManager::instance()->selectedItemAncestorOfType<RimFractureModel>();
    if ( !fractureModel ) return;

    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view ) return;

    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( view );
    if ( !eclipseView ) return;

    RimEclipseCase* eclipseCase = eclipseView->eclipseCase();
    if ( !eclipseCase ) return;

    createPlot( eclipseCase, fractureModel, view->currentTimeStep() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Fracture Model Plot" );
    // actionToSetup->setIcon( QIcon( ":/WellBoreStability16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::createFormationTrack( RimFractureModelPlot* plot,
                                                           RimFractureModel*     fractureModel,
                                                           RimEclipseCase*       eclipseCase )
{
    RimWellLogTrack* formationTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Formations", plot );
    formationTrack->setFormationWellPath( fractureModel->thicknessDirectionWellPath() );
    formationTrack->setFormationCase( eclipseCase );
    formationTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS );
    formationTrack->setVisibleXRange( 0.0, 0.0 );
    formationTrack->setOverburdenHeight( fractureModel->overburdenHeight() );
    formationTrack->setUnderburdenHeight( fractureModel->underburdenHeight() );
    formationTrack->setColSpan( RimPlot::ONE );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::createFaciesTrack( RimFractureModelPlot* plot,
                                                        RimFractureModel*     fractureModel,
                                                        RimEclipseCase*       eclipseCase )
{
    QString defaultProperty = "OPERNUM_1";

    RimWellLogTrack* faciesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Facies", plot );
    faciesTrack->setFormationWellPath( fractureModel->thicknessDirectionWellPath() );
    faciesTrack->setFormationCase( eclipseCase );
    faciesTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS );
    faciesTrack->setRegionPropertyResultType( RiaDefines::ResultCatType::INPUT_PROPERTY, defaultProperty );
    faciesTrack->setOverburdenHeight( fractureModel->overburdenHeight() );
    faciesTrack->setUnderburdenHeight( fractureModel->underburdenHeight() );

    RimColorLegend* faciesColors =
        RimProject::current()->colorLegendCollection()->findByName( RiaDefines::faciesColorLegendName() );
    if ( faciesColors )
    {
        faciesTrack->setColorShadingLegend( faciesColors );
    }

    faciesTrack->setVisibleXRange( 0.0, 0.0 );
    faciesTrack->setColSpan( RimPlot::ONE );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimFractureModelCurve* curve = new RimFractureModelCurve;
    curve->setFractureModel( fractureModel );
    curve->setCurveProperty( RiaDefines::CurveProperty::FACIES );
    curve->setCase( eclipseCase );
    curve->setEclipseResultCategory( RiaDefines::ResultCatType::INPUT_PROPERTY );
    curve->setEclipseResultVariable( defaultProperty );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
    curve->setLineThickness( 2 );
    curve->setAutoNameComponents( false, true, false, false, false );

    faciesTrack->addCurve( curve );
    faciesTrack->setAutoScaleXEnabled( true );
    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();
    faciesTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaApplication::instance()->project()->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::createLayersTrack( RimFractureModelPlot* plot,
                                                        RimFractureModel*     fractureModel,
                                                        RimEclipseCase*       eclipseCase )
{
    QString defaultProperty = "OPERNUM_1";

    RimWellLogTrack* faciesTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Layers", plot );
    faciesTrack->setFormationWellPath( fractureModel->thicknessDirectionWellPath() );
    faciesTrack->setFormationCase( eclipseCase );

    RimColorLegend* faciesColors =
        RimProject::current()->colorLegendCollection()->findByName( RiaDefines::faciesColorLegendName() );
    if ( faciesColors )
    {
        faciesTrack->setColorShadingLegend( faciesColors );
    }

    faciesTrack->setVisibleXRange( 0.0, 0.0 );
    faciesTrack->setColSpan( RimPlot::ONE );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimLayerCurve* curve = new RimLayerCurve;
    curve->setCurveProperty( RiaDefines::CurveProperty::LAYERS );
    curve->setFractureModel( fractureModel );
    curve->setCase( eclipseCase );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
    curve->setLineThickness( 2 );
    curve->setAutoNameComponents( false, true, false, false, false );

    faciesTrack->addCurve( curve );
    faciesTrack->setAutoScaleXEnabled( true );
    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();
    faciesTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaApplication::instance()->project()->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::createParametersTrack( RimFractureModelPlot* plot,
                                                            RimFractureModel*     fractureModel,
                                                            RimEclipseCase*       eclipseCase,
                                                            int                   timeStep,
                                                            const QString&        trackTitle,
                                                            const PlotDefVector&  curveConfigurations,
                                                            bool                  isPlotLogarithmic )
{
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, trackTitle, plot );
    plotTrack->setFormationCase( eclipseCase );
    plotTrack->setFormationWellPath( fractureModel->thicknessDirectionWellPath() );
    plotTrack->setColSpan( RimPlot::TWO );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    int colorIndex = 0;
    for ( auto curveConfig : curveConfigurations )
    {
        QString                                     resultVariable       = std::get<0>( curveConfig );
        RiaDefines::ResultCatType                   resultCategoryType   = std::get<1>( curveConfig );
        RimFractureModelCurve::MissingValueStrategy missingValueStrategy = std::get<2>( curveConfig );
        bool                                        fixedInitialTimeStep = std::get<3>( curveConfig );
        RiaDefines::CurveProperty                   curveProperty        = std::get<4>( curveConfig );

        RimFractureModelCurve* curve = new RimFractureModelCurve;
        curve->setCurveProperty( curveProperty );
        curve->setFractureModel( fractureModel );
        curve->setCase( eclipseCase );
        curve->setEclipseResultVariable( resultVariable );
        curve->setEclipseResultCategory( resultCategoryType );
        curve->setMissingValueStrategy( missingValueStrategy );
        curve->setColor( colors.cycledColor3f( colorIndex ) );
        curve->setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
        curve->setLineThickness( 2 );

        if ( fixedInitialTimeStep )
        {
            curve->setAutoNameComponents( false, false, false, false, false );
            curve->setCustomName( QString( "INITIAL %1" ).arg( resultVariable ) );
            curve->setCurrentTimeStep( 0 );
        }
        else
        {
            curve->setAutoNameComponents( false, true, false, false, false );
            curve->setCurrentTimeStep( timeStep );
        }

        if ( curveProperty == RiaDefines::CurveProperty::INITIAL_PRESSURE )
        {
            curve->setBurdenStrategy( RimFractureModelCurve::BurdenStrategy::GRADIENT );
        }

        plotTrack->addCurve( curve );
        curve->loadDataAndUpdate( true );

        curve->updateConnectedEditors();

        colorIndex++;
    }

    plotTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setLogarithmicScale( isPlotLogarithmic );
    plotTrack->setAutoScaleXEnabled( true );
    plotTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaApplication::instance()->project()->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    // RiuPlotMainWindowTools::selectAsCurrentItem( curve );
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::createElasticPropertiesTrack( RimFractureModelPlot*     plot,
                                                                   RimFractureModel*         fractureModel,
                                                                   RimEclipseCase*           eclipseCase,
                                                                   int                       timeStep,
                                                                   RiaDefines::CurveProperty propertyType )
{
    QString          trackName = caf::AppEnum<RiaDefines::CurveProperty>::uiText( propertyType );
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, trackName, plot );
    plotTrack->setFormationWellPath( fractureModel->thicknessDirectionWellPath() );
    plotTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    plotTrack->setLogarithmicScale( false );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setShowWindow( true );
    plotTrack->setColSpan( RimPlot::TWO );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimElasticPropertiesCurve* curve = new RimElasticPropertiesCurve;
    curve->setCurveProperty( propertyType );
    curve->setFractureModel( fractureModel );
    curve->setCase( eclipseCase );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
    curve->setLineThickness( 2 );
    curve->setUiName( trackName );
    curve->setAutoNameComponents( false, false, false, false, false );

    plotTrack->addCurve( curve );
    plotTrack->setAutoScaleXEnabled( true );
    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();
    plotTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaApplication::instance()->project()->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewFractureModelPlotFeature::createStressTrack( RimFractureModelPlot*     plot,
                                                        RimFractureModel*         fractureModel,
                                                        RimEclipseCase*           eclipseCase,
                                                        int                       timeStep,
                                                        RiaDefines::CurveProperty propertyType )
{
    QString          trackName = caf::AppEnum<RiaDefines::CurveProperty>::uiText( propertyType );
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, trackName, plot );
    plotTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR );
    plotTrack->setLogarithmicScale( false );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setShowWindow( true );
    plotTrack->setColSpan( RimPlot::TWO );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimFractureModelStressCurve* curve = new RimFractureModelStressCurve;
    curve->setCurveProperty( propertyType );
    curve->setFractureModel( fractureModel );
    curve->setCase( eclipseCase );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
    curve->setLineThickness( 2 );
    curve->setUiName( trackName );
    curve->setAutoNameComponents( false, false, false, false, false );
    if ( propertyType == RiaDefines::CurveProperty::STRESS_GRADIENT )
    {
        curve->setInterpolation( RiuQwtPlotCurve::INTERPOLATION_STEP_LEFT );
    }

    plotTrack->addCurve( curve );
    plotTrack->setAutoScaleXEnabled( true );
    curve->loadDataAndUpdate( true );

    curve->updateConnectedEditors();
    plotTrack->updateConnectedEditors();
    plot->updateConnectedEditors();

    RiaApplication::instance()->project()->updateConnectedEditors();

    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();
    RiuPlotMainWindowTools::selectAsCurrentItem( curve );
    RiuPlotMainWindowTools::showPlotMainWindow();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlot* RicNewFractureModelPlotFeature::createFractureModelPlot( bool           showAfterCreation,
                                                                               const QString& plotDescription )

{
    RimFractureModelPlotCollection* fractureModelPlotColl = fractureModelPlotCollection();
    CVF_ASSERT( fractureModelPlotColl );

    // Make sure the summary plot window is created
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();

    RimFractureModelPlot* plot = new RimFractureModelPlot();
    plot->setAsPlotMdiWindow();

    fractureModelPlotColl->addFractureModelPlot( plot );

    if ( !plotDescription.isEmpty() )
    {
        plot->nameConfig()->setCustomName( plotDescription );
    }
    else
    {
        plot->nameConfig()->setCustomName(
            QString( "Fracture Model Plot %1" ).arg( fractureModelPlotCollection()->fractureModelPlots().size() ) );
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
RimFractureModelPlotCollection* RicNewFractureModelPlotFeature::fractureModelPlotCollection()
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT( project );

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT( mainPlotColl );

    RimFractureModelPlotCollection* fractureModelPlotColl = mainPlotColl->fractureModelPlotCollection();
    CVF_ASSERT( fractureModelPlotColl );

    return mainPlotColl->fractureModelPlotCollection();
}
