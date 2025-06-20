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

#include "RicNewHistogramCurveFeature.h"

#include "RiaColorTables.h"
#include "RiaGuiApplication.h"

#include "Histogram/RimEnsembleParameterHistogramDataSource.h"
#include "Histogram/RimGridStatisticsHistogramDataSource.h"
#include "Histogram/RimHistogramCurve.h"
#include "Histogram/RimHistogramCurveCollection.h"
#include "Histogram/RimHistogramPlot.h"
#include "RimProject.h"

#include "RiuPlotMainWindow.h"
#include "RiuPlotMainWindowTools.h"

#include "cafAssert.h"
#include "cafSelectionManager.h"

#include <QAction>

CAF_CMD_SOURCE_INIT( RicNewHistogramCurveFeature, "RicNewHistogramCurveFeature" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RicNewHistogramCurveFeature::isCommandEnabled() const
{
    return ( selectedHistogramPlot() );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewHistogramCurveFeature::onActionTriggered( bool isChecked )
{
    RiaGuiApplication* app     = RiaGuiApplication::instance();
    RimProject*        project = app->project();
    CAF_ASSERT( project );

    auto getDataSourceFromString = []( const QString& dataSourceType ) -> RimHistogramDataSource*
    {
        if ( dataSourceType == "Ensemble Parameter" )
            return new RimEnsembleParameterHistogramDataSource();
        else if ( dataSourceType == "Grid Statistics" )
            return new RimGridStatisticsHistogramDataSource();
        else if ( dataSourceType == "Summary Vector" )
            return nullptr;
        return nullptr;
    };

    RimHistogramPlot* plot = selectedHistogramPlot();
    if ( plot )
    {
        RimHistogramCurve* newCurve = new RimHistogramCurve();

        RimHistogramDataSource* dataSource = getDataSourceFromString( userData().toString() );
        newCurve->setDataSource( dataSource );

        cvf::Color3f curveColor = RiaColorTables::summaryCurveDefaultPaletteColors().cycledColor3f( plot->curveCount() );
        newCurve->setColor( curveColor );
        newCurve->setIsStacked( true );
        newCurve->setFillColor( curveColor );

        plot->addCurveNoUpdate( newCurve );

        plot->loadDataAndUpdate();

        RiuPlotMainWindow* mainPlotWindow = app->mainPlotWindow();
        mainPlotWindow->updateMultiPlotToolBar();

        plot->updateConnectedEditors();
        RiuPlotMainWindowTools::selectAsCurrentItem( newCurve );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RicNewHistogramCurveFeature::setupActionLook( QAction* actionToSetup )
{
    actionToSetup->setText( "New Histogram Curve" );
    actionToSetup->setIcon( QIcon( ":/SummaryCurve16x16.png" ) );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimHistogramPlot* RicNewHistogramCurveFeature::selectedHistogramPlot() const
{
    RimHistogramPlot* sumPlot = nullptr;

    caf::PdmObject* selObj = dynamic_cast<caf::PdmObject*>( caf::SelectionManager::instance()->selectedItem() );
    if ( selObj )
    {
        return selObj->firstAncestorOrThisOfType<RimHistogramPlot>();
    }

    return sumPlot;
}
