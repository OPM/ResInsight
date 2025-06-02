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

#include "Histogram/RimHistogramCurve.h"
#include "Histogram/RimHistogramDataSource.h"
#include "Histogram/RimHistogramMultiPlot.h"
#include "Histogram/RimHistogramMultiPlotCollection.h"
#include "Histogram/RimHistogramPlot.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"

#include "RiaColorTables.h"
#include "RiaGuiApplication.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

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

    cvf::Color3f curveColor = RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( plot->curveCount() );
    newCurve->setColor( curveColor );
    newCurve->setIsStacked( true );
    newCurve->setFillColor( curveColor );

    plot->addCurveNoUpdate( newCurve );

    plot->loadDataAndUpdate();
    plot->updateConnectedEditors();

    RiuPlotMainWindow* mainPlotWindow = app->mainPlotWindow();
    mainPlotWindow->updateMultiPlotToolBar();

    RiuPlotMainWindowTools::onObjectAppended( newCurve, plot );
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
