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

#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"

#include "RigWellPath.h"

#include "RimColorLegend.h"
#include "RimColorLegendCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimEclipseView.h"
#include "RimFaciesProperties.h"
#include "RimMainPlotCollection.h"
#include "RimModeledWellPath.h"
#include "RimProject.h"
#include "RimStimPlanModel.h"
#include "RimStimPlanModelCurve.h"
#include "RimStimPlanModelPlot.h"
#include "RimStimPlanModelPlotCollection.h"
#include "RimStimPlanModelTemplate.h"
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
    RimEclipseCase* eclipseCase = stimPlanModel->eclipseCase();
    int             timeStep    = stimPlanModel->timeStep();

    caf::ProgressInfo progInfo( 100, "Creating StimPlan Model Plot" );

    RimStimPlanModelPlot* plot = createStimPlanModelPlot( true, "StimPlan Model" );
    plot->setStimPlanModel( stimPlanModel );

    {
        auto task = progInfo.task( "Creating formation track", 2 );
        createFormationTrack( plot, stimPlanModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating facies track", 2 );
        createFaciesTrack( plot, stimPlanModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating layers track", 2 );
        createLayersTrack( plot, stimPlanModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating parameters track", 15 );

        std::map<QString, std::vector<RiaDefines::CurveProperty>> plots;
        plots["Porosity"]     = { RiaDefines::CurveProperty::POROSITY, RiaDefines::CurveProperty::POROSITY_UNSCALED };
        plots["Pressure"]     = { RiaDefines::CurveProperty::INITIAL_PRESSURE, RiaDefines::CurveProperty::PRESSURE };
        plots["Permeability"] = { RiaDefines::CurveProperty::PERMEABILITY_X, RiaDefines::CurveProperty::PERMEABILITY_Z };
        plots["Net-To-Gross"] = { RiaDefines::CurveProperty::NET_TO_GROSS };

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
        createParametersTrack( plot,
                               stimPlanModel,
                               eclipseCase,
                               timeStep,
                               "Stress Gradient",
                               { RiaDefines::CurveProperty::STRESS_GRADIENT } );
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
        createParametersTrack( plot,
                               stimPlanModel,
                               eclipseCase,
                               timeStep,
                               "Temperature",
                               { RiaDefines::CurveProperty::TEMPERATURE } );
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
bool RicNewStimPlanModelPlotFeature::isCommandEnabled()
{
    Rim3dView* view = RiaApplication::instance()->activeReservoirView();
    if ( !view ) return false;
    RimEclipseView* eclipseView = dynamic_cast<RimEclipseView*>( view );
    return eclipseView != nullptr;
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
void RicNewStimPlanModelPlotFeature::createFormationTrack( RimStimPlanModelPlot* plot,
                                                           RimStimPlanModel*     stimPlanModel,
                                                           RimEclipseCase*       eclipseCase )
{
    RimWellLogTrack* formationTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, "Formations", plot );
    formationTrack->setFormationWellPath( stimPlanModel->thicknessDirectionWellPath() );
    formationTrack->setFormationCase( eclipseCase );
    formationTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::FORMATION_ANNOTATIONS );
    formationTrack->setVisibleXRange( 0.0, 0.0 );
    formationTrack->setOverburdenHeight( stimPlanModel->overburdenHeight() );
    formationTrack->setUnderburdenHeight( stimPlanModel->underburdenHeight() );
    formationTrack->setColSpan( RimPlot::ONE );
    formationTrack->setLegendsVisible( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewStimPlanModelPlotFeature::createFaciesTrack( RimStimPlanModelPlot* plot,
                                                        RimStimPlanModel*     stimPlanModel,
                                                        RimEclipseCase*       eclipseCase )
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
    faciesTrack->setAnnotationType( RiuPlotAnnotationTool::RegionAnnotationType::RESULT_PROPERTY_ANNOTATIONS );
    faciesTrack->setRegionPropertyResultType( faciesDefinition->resultType(), faciesDefinition->resultVariable() );
    faciesTrack->setOverburdenHeight( stimPlanModel->overburdenHeight() );
    faciesTrack->setUnderburdenHeight( stimPlanModel->underburdenHeight() );
    faciesTrack->setLegendsVisible( true );
    faciesTrack->setPlotTitleVisible( true );

    RimColorLegend* faciesColors = faciesProperties->colorLegend();
    if ( faciesColors ) faciesTrack->setColorShadingLegend( faciesColors );

    faciesTrack->setVisibleXRange( 0.0, 0.0 );
    faciesTrack->setColSpan( RimPlot::ONE );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimStimPlanModelCurve* curve = new RimStimPlanModelCurve;
    curve->setStimPlanModel( stimPlanModel );
    curve->setCurveProperty( RiaDefines::CurveProperty::FACIES );
    curve->setCase( eclipseCase );
    curve->setEclipseResultCategory( faciesDefinition->resultType() );
    curve->setEclipseResultVariable( faciesDefinition->resultVariable() );
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
void RicNewStimPlanModelPlotFeature::createLayersTrack( RimStimPlanModelPlot* plot,
                                                        RimStimPlanModel*     stimPlanModel,
                                                        RimEclipseCase*       eclipseCase )
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

    faciesTrack->setVisibleXRange( 0.0, 0.0 );
    faciesTrack->setColSpan( RimPlot::ONE );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    RimStimPlanModelCurve* curve = new RimStimPlanModelCurve;
    curve->setCurveProperty( RiaDefines::CurveProperty::LAYERS );
    curve->setStimPlanModel( stimPlanModel );
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
void RicNewStimPlanModelPlotFeature::createParametersTrack( RimStimPlanModelPlot*                         plot,
                                                            RimStimPlanModel*                             stimPlanModel,
                                                            RimEclipseCase*                               eclipseCase,
                                                            int                                           timeStep,
                                                            const QString&                                trackTitle,
                                                            const std::vector<RiaDefines::CurveProperty>& propertyTypes,
                                                            bool isPlotLogarithmic )
{
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, trackTitle, plot );
    plotTrack->setFormationCase( eclipseCase );
    plotTrack->setFormationWellPath( stimPlanModel->thicknessDirectionWellPath() );
    plotTrack->setColSpan( RimPlot::TWO );
    plotTrack->setLegendsVisible( true );
    plotTrack->setPlotTitleVisible( true );
    plotTrack->setShowWindow( shouldShowByDefault( propertyTypes ) );

    caf::ColorTable colors = RiaColorTables::wellLogPlotPaletteColors();

    int colorIndex = 0;
    for ( const RiaDefines::CurveProperty& propertyType : propertyTypes )
    {
        QString                   resultVariable     = stimPlanModel->eclipseResultVariable( propertyType );
        RiaDefines::ResultCatType resultCategoryType = stimPlanModel->eclipseResultCategory( propertyType );
        // TODO: maybe improve?
        bool fixedInitialTimeStep = ( propertyType == RiaDefines::CurveProperty::INITIAL_PRESSURE );

        RimStimPlanModelCurve* curve = new RimStimPlanModelCurve;
        curve->setCurveProperty( propertyType );
        curve->setStimPlanModel( stimPlanModel );
        curve->setCase( eclipseCase );
        curve->setEclipseResultVariable( resultVariable );
        curve->setEclipseResultCategory( resultCategoryType );
        curve->setColor( colors.cycledColor3f( colorIndex ) );
        curve->setLineStyle( RiuQwtPlotCurve::STYLE_SOLID );
        curve->setLineThickness( 2 );

        if ( propertyType == RiaDefines::CurveProperty::STRESS_GRADIENT )
        {
            curve->setInterpolation( RiuQwtPlotCurve::INTERPOLATION_STEP_LEFT );
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
RimStimPlanModelPlot* RicNewStimPlanModelPlotFeature::createStimPlanModelPlot( bool           showAfterCreation,
                                                                               const QString& plotDescription )

{
    RimStimPlanModelPlotCollection* stimPlanModelPlotColl = stimPlanModelPlotCollection();
    CVF_ASSERT( stimPlanModelPlotColl );

    // Make sure the summary plot window is created
    RiaGuiApplication::instance()->getOrCreateMainPlotWindow();

    RimStimPlanModelPlot* plot = new RimStimPlanModelPlot();
    plot->setAsPlotMdiWindow();

    stimPlanModelPlotColl->addStimPlanModelPlot( plot );

    if ( !plotDescription.isEmpty() )
    {
        plot->nameConfig()->setCustomName( plotDescription );
    }
    else
    {
        plot->nameConfig()->setCustomName(
            QString( "StimPlan Model Plot %1" ).arg( stimPlanModelPlotCollection()->stimPlanModelPlots().size() ) );
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
RimStimPlanModelPlotCollection* RicNewStimPlanModelPlotFeature::stimPlanModelPlotCollection()
{
    RimProject* project = RiaApplication::instance()->project();
    CVF_ASSERT( project );

    RimMainPlotCollection* mainPlotColl = project->mainPlotCollection();
    CVF_ASSERT( mainPlotColl );

    RimStimPlanModelPlotCollection* stimPlanModelPlotColl = mainPlotColl->stimPlanModelPlotCollection();
    CVF_ASSERT( stimPlanModelPlotColl );

    return mainPlotColl->stimPlanModelPlotCollection();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewStimPlanModelPlotFeature::shouldShowByDefault( const std::vector<RiaDefines::CurveProperty>& propertyTypes )
{
    std::vector<RiaDefines::CurveProperty> defaultPropertyTypes = {
        RiaDefines::CurveProperty::INITIAL_PRESSURE,
        RiaDefines::CurveProperty::PRESSURE,
        RiaDefines::CurveProperty::STRESS,
        RiaDefines::CurveProperty::INITIAL_STRESS,
        RiaDefines::CurveProperty::STRESS_GRADIENT,
        RiaDefines::CurveProperty::YOUNGS_MODULUS,
        RiaDefines::CurveProperty::POISSONS_RATIO,
        RiaDefines::CurveProperty::K_IC,
        RiaDefines::CurveProperty::PROPPANT_EMBEDMENT,
        RiaDefines::CurveProperty::FLUID_LOSS_COEFFICIENT,
    };

    for ( auto propertyType : propertyTypes )
    {
        for ( auto defaultPropertyType : defaultPropertyTypes )
        {
            if ( propertyType == defaultPropertyType ) return true;
        }
    }
    return false;
}
