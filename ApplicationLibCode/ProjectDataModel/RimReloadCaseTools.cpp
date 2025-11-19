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

#include "RiaEclipseFileNameTools.h"
#include "RiaFractureDefines.h"
#include "RiaImportEclipseCaseTools.h"
#include "RiaLogging.h"
#include "RiaPreferencesGrid.h"
#include "Summary/RiaSummaryTools.h"

#include "ApplicationCommands/RicShowMainWindowFeature.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"

#include "ContourMap/RimEclipseContourMapProjection.h"
#include "ContourMap/RimEclipseContourMapView.h"
#include "ContourMap/RimEclipseContourMapViewCollection.h"
#include "Rim2dIntersectionView.h"
#include "Rim2dIntersectionViewCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseView.h"
#include "RimGridCalculation.h"
#include "RimGridCalculationCollection.h"
#include "RimMainPlotCollection.h"
#include "RimProject.h"
#include "RimSummaryCase.h"
#include "RimSummaryCaseMainCollection.h"

#include "Riu3DMainWindowTools.h"

#include <QFileInfo>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::reloadEclipseGridAndSummary( RimEclipseCase* eclipseCase )
{
    reloadEclipseData( eclipseCase, true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::reloadEclipseGrid( RimEclipseCase* eclipseCase )
{
    reloadEclipseData( eclipseCase, false );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::reloadEclipseData( RimEclipseCase* eclipseCase, bool reloadSummaryData )
{
    CVF_ASSERT( eclipseCase );

    RigEclipseCaseData* eclipseCaseData = eclipseCase->eclipseCaseData();
    if ( !eclipseCaseData ) return;

    clearAllGridData( eclipseCaseData );

    eclipseCase->reloadEclipseGridFile();

    std::vector<RimGridCalculation*> gridCalculations = RimProject::current()->gridCalculationCollection()->sortedGridCalculations();

    for ( auto gridCalculation : gridCalculations )
    {
        bool recalculate = false;
        for ( auto inputCase : gridCalculation->inputCases() )
        {
            if ( inputCase == eclipseCase ) recalculate = true;
        }

        if ( recalculate ) gridCalculation->calculate();
    }

    updateAll3dViews( eclipseCase );

    if ( reloadSummaryData )
    {
        auto summaryCase = RimReloadCaseTools::findSummaryCaseFromEclipseResultCase( dynamic_cast<RimEclipseResultCase*>( eclipseCase ) );
        RiaSummaryTools::reloadSummaryCaseAndUpdateConnectedPlots( summaryCase );
        summaryCase->updateConnectedEditors();
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

    RigCaseCellResultsData* stimPlanModelResults = eclipseCaseData->results( RiaDefines::PorosityModelType::FRACTURE_MODEL );
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

        if ( contourMap->cellResult()->resultType() == RiaDefines::ResultCatType::GENERATED )
        {
            // When a generated result is selected, the data might come from a calculation. Make sure that all
            // computations are updated based on new data.
            // See RimEclipseContourMapProjection::generateResults()
            contourMap->contourMapProjection()->clearGeometry();
            if ( auto projection = dynamic_cast<RimEclipseContourMapProjection*>( contourMap->contourMapProjection() ) )
                projection->clearGridMappingAndRedraw();
        }

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
RimEclipseCase* RimReloadCaseTools::gridModelFromSummaryCase( const RimSummaryCase* summaryCase )
{
    if ( summaryCase )
    {
        QString                 summaryFileName = summaryCase->summaryHeaderFilename();
        RiaEclipseFileNameTools fileHelper( summaryFileName );

        auto candidateGridFileName = fileHelper.findRelatedGridFile();
        return RimProject::current()->eclipseCaseFromGridFileName( candidateGridFileName );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimSummaryCase* RimReloadCaseTools::findSummaryCaseFromEclipseResultCase( const RimEclipseResultCase* eclipseResultCase )
{
    RimSummaryCaseMainCollection* sumCaseColl = RiaSummaryTools::summaryCaseMainCollection();
    if ( !sumCaseColl ) return nullptr;

    RiaEclipseFileNameTools helper( eclipseResultCase->gridFileName() );

    auto summaryFileNames = helper.findSummaryFileCandidates();
    for ( const auto& fileName : summaryFileNames )
    {
        return sumCaseColl->findTopLevelSummaryCaseFromFileName( fileName );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReloadCaseTools::openOrImportGridModelFromSummaryCase( const RimSummaryCase* summaryCase )
{
    if ( !summaryCase ) return false;

    if ( findGridModelAndActivateFirstView( summaryCase ) ) return true;

    QString                 summaryFileName = summaryCase->summaryHeaderFilename();
    RiaEclipseFileNameTools fileHelper( summaryFileName );
    auto                    candidateGridFileName = fileHelper.findRelatedGridFile();

    if ( QFileInfo::exists( candidateGridFileName ) )
    {
        bool              createView = true;
        RifReaderSettings rs         = RiaPreferencesGrid::current()->readerSettings();
        auto              id         = RiaImportEclipseCaseTools::openEclipseCaseFromFile( candidateGridFileName, createView, rs );
        if ( id > -1 )
        {
            RiaLogging::info( QString( "Imported %1" ).arg( candidateGridFileName ) );

            return true;
        }
    }

    RiaLogging::info( QString( "No grid case found based on summary file %1" ).arg( summaryFileName ) );

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimReloadCaseTools::updateAllPlots()
{
    RimMainPlotCollection::current()->loadDataAndUpdateAllPlots();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimReloadCaseTools::findGridModelAndActivateFirstView( const RimSummaryCase* summaryCase )
{
    auto gridCase = RimReloadCaseTools::gridModelFromSummaryCase( summaryCase );
    if ( gridCase )
    {
        if ( !gridCase->gridViews().empty() )
        {
            RicShowMainWindowFeature::showMainWindow();

            Riu3DMainWindowTools::selectAsCurrentItem( gridCase->gridViews().front() );
        }

        return true;
    }

    return false;
}
