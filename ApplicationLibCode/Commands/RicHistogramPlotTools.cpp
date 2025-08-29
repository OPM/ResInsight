/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "RicHistogramPlotTools.h"

#include "Histogram/RimEnsembleFractureHistogramDataSource.h"
#include "Histogram/RimEnsembleParameterHistogramDataSource.h"
#include "Histogram/RimEnsembleSummaryVectorHistogramDataSource.h"
#include "Histogram/RimGridStatisticsHistogramDataSource.h"
#include "Histogram/RimHistogramCurve.h"
#include "Histogram/RimHistogramDataSource.h"
#include "Histogram/RimHistogramMultiPlot.h"
#include "Histogram/RimHistogramMultiPlotCollection.h"
#include "Histogram/RimHistogramPlot.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryEnsemble.h"

#include "RiaColorTables.h"
#include "RiaGuiApplication.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

namespace caf
{
template <>
void caf::AppEnum<RicHistogramPlotTools::DataSourceType>::setUp()
{
    addItem( RicHistogramPlotTools::DataSourceType::ENSEMBLE_PARAMETER, "ENSEMBLE_PARAMETER", "Ensemble Parameter" );
    addItem( RicHistogramPlotTools::DataSourceType::GRID_STATISTICS, "GRID_STATISTICS", "Grid Statistics" );
    addItem( RicHistogramPlotTools::DataSourceType::SUMMARY_VECTOR, "SUMMARY_VECTOR", "Summary Vector" );
    addItem( RicHistogramPlotTools::DataSourceType::ENSEMBLE_FRACTURE_STATISTICS, "ENSEMBLE_FRACTURE_STATISTICS", "Ensemble Fracture Statistics" );
    setDefault( RicHistogramPlotTools::DataSourceType::SUMMARY_VECTOR );
}
}; // namespace caf

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RicHistogramPlotTools::DataSourceType> RicHistogramPlotTools::allDataSourceTypes()
{
    return { RicHistogramPlotTools::DataSourceType::ENSEMBLE_PARAMETER,
             RicHistogramPlotTools::DataSourceType::GRID_STATISTICS,
             RicHistogramPlotTools::DataSourceType::SUMMARY_VECTOR,
             RicHistogramPlotTools::DataSourceType::ENSEMBLE_FRACTURE_STATISTICS };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHistogramPlotTools::createDefaultHistogramCurve( RimHistogramPlot* plot, RicHistogramPlotTools::DataSourceType dataSourceType )
{
    auto getDataSourceFromType = []( DataSourceType dataSourceType ) -> RimHistogramDataSource*
    {
        if ( dataSourceType == DataSourceType::ENSEMBLE_PARAMETER )
            return new RimEnsembleParameterHistogramDataSource();
        else if ( dataSourceType == DataSourceType::GRID_STATISTICS )
            return new RimGridStatisticsHistogramDataSource();
        else if ( dataSourceType == DataSourceType::SUMMARY_VECTOR )
            return new RimEnsembleSummaryVectorHistogramDataSource();
        else if ( dataSourceType == DataSourceType::ENSEMBLE_FRACTURE_STATISTICS )
            return new RimEnsembleFractureHistogramDataSource();
        return nullptr;
    };

    RimHistogramDataSource* dataSource = getDataSourceFromType( dataSourceType );
    dataSource->setDefaults();
    return createHistogramCurve( plot, dataSource );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHistogramPlotTools::createHistogramCurve( RimHistogramPlot* plot, RimHistogramDataSource* dataSource )
{
    RiaGuiApplication* app     = RiaGuiApplication::instance();
    RimProject*        project = app->project();
    CAF_ASSERT( project );

    RimHistogramCurve* newCurve = new RimHistogramCurve();

    newCurve->setDataSource( dataSource );
    newCurve->setAppearanceFromGraphType( plot->graphType() );

    cvf::Color3f curveColor = RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( plot->curveCount() );
    newCurve->setColor( curveColor );
    newCurve->setFillColor( curveColor );

    RicHistogramPlotTools::addHistogramCurveToPlot( plot, newCurve );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlot* RicHistogramPlotTools::addNewHistogramMultiplot()
{
    auto collection = RimMainPlotCollection::current()->histogramMultiPlotCollection();
    if ( !collection ) return nullptr;

    return addNewHistogramMultiplot( collection );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramMultiPlot* RicHistogramPlotTools::addNewHistogramMultiplot( RimHistogramMultiPlotCollection* collection )
{
    CAF_ASSERT( collection );

    RimHistogramMultiPlot* multiplot = collection->appendHistogramMultiPlot();
    multiplot->setAsPlotMdiWindow();
    multiplot->setShowWindow( true );
    multiplot->loadDataAndUpdate();
    collection->updateConnectedEditors();

    RiuPlotMainWindowTools::onObjectAppended( multiplot, collection );

    return multiplot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot* RicHistogramPlotTools::addNewHistogramPlot( RimHistogramMultiPlot* histogramMultiPlot )
{
    RimHistogramPlot* plot = new RimHistogramPlot();
    plot->enableAutoPlotTitle( true );
    histogramMultiPlot->addPlot( plot );

    RiuPlotMainWindowTools::onObjectAppended( plot, histogramMultiPlot );

    return plot;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RimHistogramDataSource*> RicHistogramPlotTools::existingDataSources( RimHistogramPlot* plot )
{
    std::vector<RimHistogramDataSource*> sources;

    for ( auto curve : plot->histogramCurves() )
    {
        sources.push_back( curve->dataSource() );
    }

    return sources;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHistogramPlotTools::appendEnsembleParameterHistogramCurve( RimHistogramPlot* plot, RimEnsembleParameterHistogramDataSource* dataSource )
{
    for ( auto source : existingDataSources( plot ) )
    {
        if ( auto histSource = dynamic_cast<RimEnsembleParameterHistogramDataSource*>( source ) )
        {
            // check for duplicate
            if ( ( histSource->ensemble() == dataSource->ensemble() ) && ( histSource->ensembleParameter() == dataSource->ensembleParameter() ) )
            {
                return;
            }
        }
    }

    createHistogramCurve( plot, dataSource );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHistogramPlotTools::addHistogramCurveToPlot( RimHistogramPlot* plot, RimHistogramCurve* curve, bool resolveRefs )
{
    RiaGuiApplication* app     = RiaGuiApplication::instance();
    RimProject*        project = app->project();
    if ( project == nullptr ) return;

    plot->addCurveNoUpdate( curve );

    if ( resolveRefs ) curve->resolveReferencesRecursively();

    plot->loadDataAndUpdate();
    plot->updateConnectedEditors();

    RiuPlotMainWindow* mainPlotWindow = app->mainPlotWindow();
    mainPlotWindow->updateMultiPlotToolBar();

    RiuPlotMainWindowTools::onObjectAppended( curve, plot );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<QString> RicHistogramPlotTools::existingEnsembleParameters( RimHistogramPlot* plot )
{
    std::set<QString> foundParameters;

    for ( auto source : existingDataSources( plot ) )
    {
        if ( auto histSource = dynamic_cast<RimEnsembleParameterHistogramDataSource*>( source ) )
        {
            foundParameters.insert( histSource->ensembleParameter() );
        }
    }

    return foundParameters;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicHistogramPlotTools::appendEnsembleToHistogram( RimHistogramPlot* plot, RimSummaryEnsemble* ensemble )
{
    auto currentParameters = existingEnsembleParameters( plot );

    if ( currentParameters.empty() )
    {
        auto newDataSource = new RimEnsembleParameterHistogramDataSource();
        newDataSource->setDefaults();
        newDataSource->setEnsemble( ensemble );
        appendEnsembleParameterHistogramCurve( plot, newDataSource );
        return;
    }

    for ( auto& parameter : currentParameters )
    {
        auto newDataSource = new RimEnsembleParameterHistogramDataSource();
        newDataSource->setEnsemble( ensemble );
        newDataSource->setEnsembleParameter( parameter );
        appendEnsembleParameterHistogramCurve( plot, newDataSource );
    }
}