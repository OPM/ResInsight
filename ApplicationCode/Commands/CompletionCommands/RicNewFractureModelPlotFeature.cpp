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
#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "WellLogCommands/RicNewWellLogPlotFeatureImpl.h"

#include "RigWellPath.h"

#include "RimEclipseCase.h"
#include "RimEclipseView.h"
#include "RimFaciesPropertiesCurve.h"
#include "RimFractureModel.h"
#include "RimFractureModelCurve.h"
#include "RimFractureModelPlot.h"
#include "RimFractureModelPlotCollection.h"
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

CAF_CMD_SOURCE_INIT( RicNewFractureModelPlotFeature, "RicNewFractureModelPlotFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimFractureModelPlot*
    RicNewFractureModelPlotFeature::createPlot( RimEclipseCase* eclipseCase, RimFractureModel* fractureModel, int timeStep )

{
    caf::ProgressInfo progInfo( 100, "Creating Fracture Model Plot" );

    RimFractureModelPlot* plot = createFractureModelPlot( true, "Fracture Model" );

    {
        auto task = progInfo.task( "Creating formation track", 2 );
        createFormationTrack( plot, fractureModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating facies track", 2 );
        createFaciesTrack( plot, fractureModel, eclipseCase );
    }

    {
        auto task = progInfo.task( "Creating parameters track", 15 );

        std::vector<std::pair<QString, RiaDefines::ResultCatType>> results =
            {std::make_pair( "PORO", RiaDefines::ResultCatType::STATIC_NATIVE ),
             std::make_pair( "PRESSURE", RiaDefines::ResultCatType::DYNAMIC_NATIVE ),
             std::make_pair( "PERMZ", RiaDefines::ResultCatType::STATIC_NATIVE )};

        for ( auto result : results )
        {
            createParametersTrack( plot, fractureModel, eclipseCase, timeStep, result.first, result.second );
        }
    }

    {
        auto task = progInfo.task( "Creating facies properties track", 15 );

        std::vector<RimFaciesPropertiesCurve::PropertyType> results =
            { RimFaciesPropertiesCurve::PropertyType::YOUNGS_MODULUS,
              RimFaciesPropertiesCurve::PropertyType::POISSONS_RATIO,
              RimFaciesPropertiesCurve::PropertyType::K_IC,
              RimFaciesPropertiesCurve::PropertyType::PROPPANT_EMBEDMENT };

        for ( auto result : results )
        {
            createFaciesPropertiesTrack( plot, fractureModel, eclipseCase, timeStep, result );
        }
    }

    {
        auto task = progInfo.task( "Updating all tracks", 5 );

        plot->nameConfig()->setAutoNameTags( false, false, false, false, false );
        plot->setPlotTitleVisible( true );
        plot->setLegendsVisible( true );
        plot->setLegendsHorizontal( true );
        plot->setDepthType( RiaDefines::DepthTypeEnum::TRUE_VERTICAL_DEPTH );
        plot->setAutoScaleDepthEnabled( true );

        // RicNewWellLogPlotFeatureImpl::updateAfterCreation( plot );
        plot->loadDataAndUpdate();
    }

    RiuPlotMainWindowTools::selectAsCurrentItem( plot );

    // Make sure the summary plot window is visible
    RiuPlotMainWindowTools::showPlotMainWindow();

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
    faciesTrack->setVisibleXRange( 0.0, 0.0 );
    faciesTrack->setColSpan( RimPlot::ONE );

    caf::ColorTable                             colors     = RiaColorTables::contrastCategoryPaletteColors();
    std::vector<RiuQwtPlotCurve::LineStyleEnum> lineStyles = {RiuQwtPlotCurve::STYLE_SOLID,
                                                              RiuQwtPlotCurve::STYLE_DASH,
                                                              RiuQwtPlotCurve::STYLE_DASH_DOT};

    RimFractureModelCurve* curve = new RimFractureModelCurve;
    curve->setFractureModel( fractureModel );
    curve->setCase( eclipseCase );
    curve->setEclipseResultCategory( RiaDefines::ResultCatType::INPUT_PROPERTY );
    curve->setEclipseResultVariable( defaultProperty );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( lineStyles[0] );
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
void RicNewFractureModelPlotFeature::createParametersTrack( RimFractureModelPlot*     plot,
                                                            RimFractureModel*         fractureModel,
                                                            RimEclipseCase*           eclipseCase,
                                                            int                       timeStep,
                                                            const QString&            resultVariable,
                                                            RiaDefines::ResultCatType resultCategoryType )
{
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, resultVariable, plot );
    plotTrack->setFormationWellPath( fractureModel->thicknessDirectionWellPath() );
    plotTrack->setColSpan( RimPlot::TWO );
    plotTrack->setVisibleXRange( 0.0, 2.0 );
    plotTrack->setAutoScaleXEnabled( true );
    plotTrack->setTickIntervals( 1.0, 0.2 );
    plotTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setShowWindow( true );

    caf::ColorTable                             colors     = RiaColorTables::contrastCategoryPaletteColors();
    std::vector<RiuQwtPlotCurve::LineStyleEnum> lineStyles = {RiuQwtPlotCurve::STYLE_SOLID,
                                                              RiuQwtPlotCurve::STYLE_DASH,
                                                              RiuQwtPlotCurve::STYLE_DASH_DOT};

    RimFractureModelCurve* curve = new RimFractureModelCurve;
    curve->setFractureModel( fractureModel );
    curve->setCase( eclipseCase );
    curve->setEclipseResultVariable( resultVariable );
    curve->setEclipseResultCategory( resultCategoryType );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( lineStyles[0] );
    curve->setLineThickness( 2 );
    curve->setAutoNameComponents( false, true, false, false, false );

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
void RicNewFractureModelPlotFeature::createFaciesPropertiesTrack( RimFractureModelPlot*                  plot,
                                                                  RimFractureModel*                      fractureModel,
                                                                  RimEclipseCase*                        eclipseCase,
                                                                  int                                    timeStep,
                                                                  RimFaciesPropertiesCurve::PropertyType propertyType )
{
    QString          trackName = caf::AppEnum<RimFaciesPropertiesCurve::PropertyType>::uiText( propertyType );
    RimWellLogTrack* plotTrack = RicNewWellLogPlotFeatureImpl::createWellLogPlotTrack( false, trackName, plot );
    plotTrack->setFormationWellPath( fractureModel->thicknessDirectionWellPath() );
    plotTrack->setColSpan( RimPlot::TWO );
    plotTrack->setVisibleXRange( 0.0, 2.0 );
    plotTrack->setAutoScaleXEnabled( true );
    plotTrack->setTickIntervals( 1.0, 0.2 );
    plotTrack->setXAxisGridVisibility( RimWellLogPlot::AXIS_GRID_MAJOR_AND_MINOR );
    plotTrack->setShowRegionLabels( true );
    plotTrack->setShowWindow( true );

    caf::ColorTable                             colors     = RiaColorTables::contrastCategoryPaletteColors();
    std::vector<RiuQwtPlotCurve::LineStyleEnum> lineStyles = { RiuQwtPlotCurve::STYLE_SOLID,
                                                               RiuQwtPlotCurve::STYLE_DASH,
                                                               RiuQwtPlotCurve::STYLE_DASH_DOT };

    RimFaciesPropertiesCurve* curve = new RimFaciesPropertiesCurve;
    curve->setPropertyType( propertyType );
    curve->setFractureModel( fractureModel );
    curve->setCase( eclipseCase );
    // curve->setEclipseResultVariable( resultVariable );
    // curve->setEclipseResultCategory( resultCategoryType );
    curve->setColor( colors.cycledColor3f( 0 ) );
    curve->setLineStyle( lineStyles[0] );
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
