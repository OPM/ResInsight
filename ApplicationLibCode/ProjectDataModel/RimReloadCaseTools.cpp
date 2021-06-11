/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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

#include "RimReloadCaseTools.h"

#include "RiaFractureDefines.h"
#include "RiaSummaryTools.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseContourMapView.h"
#include "RimEclipseContourMapViewCollection.h"
#include "RimEclipseView.h"
#include "RimFlowPlotCollection.h"
#include "RimGridCrossPlot.h"
#include "RimGridCrossPlotCollection.h"
#include "RimMainPlotCollection.h"
#include "RimMultiPlot.h"
#include "RimMultiPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCaseMainCollection.h"
#include "RimSummaryPlot.h"
#include "RimSummaryPlotCollection.h"
#include "RimWellLogPlot.h"
#include "RimWellLogPlotCollection.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::reloadAllEclipseData( RimEclipseCase* eclipseCase )
{
    reloadAllEclipseData( eclipseCase, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::reloadAllEclipseGridData( RimEclipseCase* eclipseCase )
{
    reloadAllEclipseData( eclipseCase, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::reloadAllEclipseData( RimEclipseCase* eclipseCase, bool reloadSummaryData )
{
    CVF_ASSERT( eclipseCase );

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    clearAllGridData( eclipseCaseData );

    eclipseCase->reloadEclipseGridFile();

    updateAll3dViews( eclipseCase );

    if ( reloadSummaryData )
    {
        RimSummaryCaseMainCollection* sumCaseColl = RiaSummaryTools::summaryCaseMainCollection();
        if ( sumCaseColl )
        {
            sumCaseColl->loadAllSummaryCaseData();
        }
    }

    updateAllPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::clearAllGridData( RigEclipseCaseData* eclipseCaseData )
{
    if ( !eclipseCaseData ) return;

    RigCaseCellResultsData* matrixModelResults = eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    if ( matrixModelResults )
    {
        matrixModelResults->clearAllResults();
    }

    RigCaseCellResultsData* stimPlanModelResults =
        eclipseCaseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL );
    if ( stimPlanModelResults )
    {
        stimPlanModelResults->clearAllResults();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::updateAll3dViews( RimEclipseCase* eclipseCase )
{
    if ( !eclipseCase ) return;

    for ( RimEclipseView* reservoirView : eclipseCase->reservoirViews() )
    {
        CVF_ASSERT( reservoirView );
        reservoirView->setEclipseCase( eclipseCase );
        reservoirView->loadDataAndUpdate();
        reservoirView->updateGridBoxData();
        reservoirView->updateAnnotationItems();
    }

    for ( RimEclipseContourMapView* contourMap : eclipseCase->contourMapCollection()->views() )
    {
        CVF_ASSERT( contourMap );
        contourMap->loadDataAndUpdate();
        contourMap->updateGridBoxData();
        contourMap->updateAnnotationItems();
    }

    for ( Rim2dIntersectionView* view : eclipseCase->intersectionViewCollection()->views() )
    {
        view->createDisplayModelAndRedraw();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::updateAllPlots()
{
    RimProject* project = RimProject::current();
    if ( project && project->mainPlotCollection() )
    {
        RimWellLogPlotCollection* wellPlotCollection = project->mainPlotCollection()->wellLogPlotCollection();

        if ( wellPlotCollection )
        {
            for ( RimWellLogPlot* wellLogPlot : wellPlotCollection->wellLogPlots() )
            {
                wellLogPlot->loadDataAndUpdate();
            }
        }

        RimSummaryPlotCollection* summaryPlotCollection = project->mainPlotCollection()->summaryPlotCollection();
        if ( summaryPlotCollection )
        {
            for ( RimSummaryPlot* summaryPlot : summaryPlotCollection->plots() )
            {
                summaryPlot->loadDataAndUpdate();
            }
        }

        RimGridCrossPlotCollection* gridCrossPlotCollection = project->mainPlotCollection()->gridCrossPlotCollection();
        if ( gridCrossPlotCollection )
        {
            for ( RimGridCrossPlot* crossPlot : gridCrossPlotCollection->plots() )
            {
                crossPlot->loadDataAndUpdate();
            }
        }

        RimFlowPlotCollection* flowPlotCollection = project->mainPlotCollection()->flowPlotCollection();
        if ( flowPlotCollection )
        {
            flowPlotCollection->loadDataAndUpdateAllPlots();
        }

        RimMultiPlotCollection* multiPlotCollection = project->mainPlotCollection()->multiPlotCollection();
        if ( multiPlotCollection )
        {
            for ( RimMultiPlot* plotWindow : multiPlotCollection->multiPlots() )
            {
                plotWindow->loadDataAndUpdate();
            }
        }
    }
}
