/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2011-     Statoil ASA
//  Copyright (C) 2013-     Ceetron Solutions AS
//  Copyright (C) 2011-2012 Ceetron AS
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

#include "RimIdenticalGridCaseGroup.h"

#include "RiaGuiApplication.h"
#include "RiaLogging.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigGridManager.h"
#include "RigMainGrid.h"

#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseResultCase.h"
#include "RimEclipseStatisticsCase.h"
#include "RimEclipseView.h"
#include "RimReservoirCellResultsStorage.h"

#include "Riu3DMainWindowTools.h"

#include "cafPdmFieldScriptingCapability.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafProgressInfo.h"

#include <QDir>

CAF_PDM_SOURCE_INIT( RimIdenticalGridCaseGroup, "RimIdenticalGridCaseGroup" );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup::RimIdenticalGridCaseGroup()
{
    CAF_PDM_InitScriptableObjectWithNameAndComment( "Grid Case Group", ":/GridCaseGroup16x16.png", "", "", "GridCaseGroup", "A statistics case group" );

    CAF_PDM_InitScriptableField( &name, "UserDescription", QString( "Grid Case Group" ), "Name" );

    CAF_PDM_InitScriptableField( &groupId, "GroupId", -1, "Case Group ID" );
    groupId.uiCapability()->setUiReadOnly( true );
    groupId.capability<caf::PdmAbstractFieldScriptingCapability>()->setIOWriteable( false );

    CAF_PDM_InitFieldNoDefault( &statisticsCaseCollection, "StatisticsCaseCollection", "statisticsCaseCollection ChildArrayField" );

    CAF_PDM_InitFieldNoDefault( &caseCollection, "CaseCollection", "Source Cases ChildArrayField" );

    caseCollection = new RimCaseCollection;
    caseCollection->uiCapability()->setUiName( "Source Cases" );
    caseCollection->uiCapability()->setUiIconFromResourceString( ":/Cases16x16.png" );

    statisticsCaseCollection = new RimCaseCollection;
    statisticsCaseCollection->uiCapability()->setUiName( "Derived Statistics" );
    statisticsCaseCollection->uiCapability()->setUiIconFromResourceString( ":/Histograms16x16.png" );

    m_mainGrid = nullptr;

    m_unionOfMatrixActiveCells   = new RigActiveCellInfo;
    m_unionOfFractureActiveCells = new RigActiveCellInfo;

    setDeletable( true );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimIdenticalGridCaseGroup::~RimIdenticalGridCaseGroup()
{
    m_mainGrid = nullptr;

    delete caseCollection;
    caseCollection = nullptr;

    delete statisticsCaseCollection;
    statisticsCaseCollection = nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::addCase( RimEclipseCase* reservoir )
{
    CVF_ASSERT( reservoir );

    if ( !reservoir ) return;

    if ( !m_mainGrid )
    {
        m_mainGrid = reservoir->eclipseCaseData()->mainGrid();
    }
    else
    {
        reservoir->eclipseCaseData()->setMainGrid( m_mainGrid );
    }

    caseCollection()->reservoirs().push_back( reservoir );

    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::removeCase( RimEclipseCase* reservoir )
{
    if ( caseCollection()->reservoirs().count( reservoir ) == 0 )
    {
        return;
    }

    caseCollection()->reservoirs().removeChild( reservoir );

    if ( caseCollection()->reservoirs().empty() )
    {
        m_mainGrid = nullptr;
    }

    clearActiveCellUnions();
    clearStatisticsResults();
    updateMainGridAndActiveCellsForStatisticsCases();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigMainGrid* RimIdenticalGridCaseGroup::mainGrid()
{
    if ( m_mainGrid ) return m_mainGrid;

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmFieldHandle* RimIdenticalGridCaseGroup::userDescriptionField()
{
    return &name;
}

//--------------------------------------------------------------------------------------------------
///  Make sure changes in this functions is validated to RiaApplication::addEclipseCases()
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::loadMainCaseAndActiveCellInfo()
{
    if ( caseCollection()->reservoirs().empty() )
    {
        return;
    }

    // Read the main case completely including grid.
    // The mainGrid from the first case is reused directly in for the other cases.
    // When reading active cell info, only the total cell count is tested for consistency

    RimEclipseCase* mainCase = caseCollection()->reservoirs[0];
    if ( !mainCase->openReserviorCase() )
    {
        QString errorMessage = QString( "Could not open the Eclipse Grid file: \n" ) + mainCase->gridFileName() + "\n" +
                               "Current working directory is: \n" + QDir::currentPath();

        RiaLogging::errorInMessageBox( Riu3DMainWindowTools::mainWindowWidget(), "Error when opening project file", errorMessage );
        return;
    }

    RigEclipseCaseData* rigCaseData = mainCase->eclipseCaseData();
    CVF_ASSERT( rigCaseData );

    RiaDefines::PorosityModelType poroModel = RiaDefines::PorosityModelType::MATRIX_MODEL;
    mainCase->results( poroModel )->createPlaceholderResultEntries();

    // Action A : Read active cell info
    // Read active cell info from all source cases. The file access is optimized for this purpose, and result meta data
    // is copied from main case to all other cases (see "Action B")

    caf::ProgressInfo info( caseCollection()->reservoirs.size(), "Case group - Reading Active Cell data" );
    for ( size_t i = 1; i < caseCollection()->reservoirs.size(); i++ )
    {
        RimEclipseResultCase* rimReservoir = dynamic_cast<RimEclipseResultCase*>( caseCollection()->reservoirs[i] );
        if ( !rimReservoir ) continue; // Input reservoir

        if ( !rimReservoir->openAndReadActiveCellData( rigCaseData ) )
        {
            // Error message
            continue;
        }

        info.incrementProgress();
    }

    m_mainGrid = rigCaseData->mainGrid();

    // Check if we need to calculate the union of the active cells

    bool foundResultsInCache = false;
    for ( size_t i = 0; i < statisticsCaseCollection()->reservoirs.size(); i++ )
    {
        RimEclipseCase* rimReservoir = statisticsCaseCollection()->reservoirs[i];

        // Check if any results are stored in cache
        if ( rimReservoir->resultsStorage( RiaDefines::PorosityModelType::MATRIX_MODEL )->storedResultsCount() > 0 ||
             rimReservoir->resultsStorage( RiaDefines::PorosityModelType::FRACTURE_MODEL )->storedResultsCount() > 0 )
        {
            foundResultsInCache = true;
            break;
        }
    }

    if ( foundResultsInCache )
    {
        computeUnionOfActiveCells();
    }

    RigCaseCellResultsData::copyResultsMetaDataFromMainCase( rigCaseData, poroModel, caseCollection->reservoirs.childrenByType() );

    // "Load" the statistical cases

    for ( size_t i = 0; i < statisticsCaseCollection()->reservoirs.size(); i++ )
    {
        RimEclipseCase* rimReservoir = statisticsCaseCollection()->reservoirs[i];

        rimReservoir->openEclipseGridFile();

        if ( i == 0 )
        {
            rimReservoir->computeActiveCellsBoundingBox();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::computeUnionOfActiveCells()
{
    if ( m_unionOfMatrixActiveCells->reservoirActiveCellCount() > 0 )
    {
        return;
    }

    if ( caseCollection->reservoirs.empty() || !m_mainGrid )
    {
        clearActiveCellUnions();

        return;
    }

    m_unionOfMatrixActiveCells->setReservoirCellCount( m_mainGrid->globalCellArray().size() );
    m_unionOfFractureActiveCells->setReservoirCellCount( m_mainGrid->globalCellArray().size() );
    m_unionOfMatrixActiveCells->setGridCount( m_mainGrid->gridCount() );
    m_unionOfFractureActiveCells->setGridCount( m_mainGrid->gridCount() );

    size_t globalActiveMatrixIndex   = 0;
    size_t globalActiveFractureIndex = 0;

    for ( size_t gridIdx = 0; gridIdx < m_mainGrid->gridCount(); gridIdx++ )
    {
        RigGridBase* grid = m_mainGrid->gridByIndex( gridIdx );

        std::vector<char> activeM( grid->cellCount(), 0 );
        std::vector<char> activeF( grid->cellCount(), 0 );

        for ( size_t gridLocalCellIndex = 0; gridLocalCellIndex < grid->cellCount(); gridLocalCellIndex++ )
        {
            for ( size_t caseIdx = 0; caseIdx < caseCollection->reservoirs.size(); caseIdx++ )
            {
                size_t reservoirCellIndex = grid->reservoirCellIndex( gridLocalCellIndex );

                if ( activeM[gridLocalCellIndex] == 0 )
                {
                    if ( caseCollection->reservoirs[caseIdx]
                             ->eclipseCaseData()
                             ->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL )
                             ->isActive( reservoirCellIndex ) )
                    {
                        activeM[gridLocalCellIndex] = 1;
                    }
                }

                if ( activeF[gridLocalCellIndex] == 0 )
                {
                    if ( caseCollection->reservoirs[caseIdx]
                             ->eclipseCaseData()
                             ->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL )
                             ->isActive( reservoirCellIndex ) )
                    {
                        activeF[gridLocalCellIndex] = 1;
                    }
                }
            }
        }

        size_t activeMatrixIndex   = 0;
        size_t activeFractureIndex = 0;

        for ( size_t gridLocalCellIndex = 0; gridLocalCellIndex < grid->cellCount(); gridLocalCellIndex++ )
        {
            size_t reservoirCellIndex = grid->reservoirCellIndex( gridLocalCellIndex );

            if ( activeM[gridLocalCellIndex] != 0 )
            {
                m_unionOfMatrixActiveCells->setCellResultIndex( reservoirCellIndex, globalActiveMatrixIndex++ );
                activeMatrixIndex++;
            }

            if ( activeF[gridLocalCellIndex] != 0 )
            {
                m_unionOfFractureActiveCells->setCellResultIndex( reservoirCellIndex, globalActiveFractureIndex++ );
                activeFractureIndex++;
            }
        }

        m_unionOfMatrixActiveCells->setGridActiveCellCounts( gridIdx, activeMatrixIndex );
        m_unionOfFractureActiveCells->setGridActiveCellCounts( gridIdx, activeFractureIndex );
    }

    m_unionOfMatrixActiveCells->computeDerivedData();
    m_unionOfFractureActiveCells->computeDerivedData();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RimIdenticalGridCaseGroup::createAndAppendStatisticsCase()
{
    bool selectDefaultResults = true;
    return createStatisticsCase( selectDefaultResults );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RimIdenticalGridCaseGroup::createAndAppendEmptyStatisticsCase()
{
    bool selectDefaultResults = false;
    return createStatisticsCase( selectDefaultResults );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::updateMainGridAndActiveCellsForStatisticsCases()
{
    for ( size_t i = 0; i < statisticsCaseCollection->reservoirs().size(); i++ )
    {
        RimEclipseCase* rimStaticsCase = statisticsCaseCollection->reservoirs[i];

        if ( rimStaticsCase->eclipseCaseData() )
        {
            rimStaticsCase->eclipseCaseData()->setMainGrid( mainGrid() );

            if ( i == 0 )
            {
                rimStaticsCase->computeActiveCellsBoundingBox();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::clearStatisticsResults()
{
    for ( size_t i = 0; i < statisticsCaseCollection->reservoirs().size(); i++ )
    {
        RimEclipseCase* rimStaticsCase = statisticsCaseCollection->reservoirs[i];
        if ( !rimStaticsCase ) continue;

        if ( rimStaticsCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL ) )
        {
            rimStaticsCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->clearAllResults();
        }
        if ( rimStaticsCase->results( RiaDefines::PorosityModelType::FRACTURE_MODEL ) )
        {
            rimStaticsCase->results( RiaDefines::PorosityModelType::FRACTURE_MODEL )->clearAllResults();
        }

        for ( size_t j = 0; j < rimStaticsCase->reservoirViews.size(); j++ )
        {
            RimEclipseView* rimReservoirView = rimStaticsCase->reservoirViews[j];
            rimReservoirView->cellResult()->setResultVariable( RiaResultNames::undefinedResultName() );
            rimReservoirView->cellEdgeResult()->setResultVariable( RiaResultNames::undefinedResultName() );
            rimReservoirView->loadDataAndUpdate();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RimIdenticalGridCaseGroup::clearActiveCellUnions()
{
    m_unionOfMatrixActiveCells->clear();
    m_unionOfFractureActiveCells->clear();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseStatisticsCase* RimIdenticalGridCaseGroup::createStatisticsCase( bool selectDefaultResults )
{
    RimEclipseStatisticsCase* newStatisticsCase = new RimEclipseStatisticsCase;

    newStatisticsCase->setCaseUserDescription( QString( "Statistics " ) + QString::number( statisticsCaseCollection()->reservoirs.size() + 1 ) );
    statisticsCaseCollection()->reservoirs.push_back( newStatisticsCase );

    if ( selectDefaultResults ) newStatisticsCase->populateResultSelectionAfterLoadingGrid();

    auto reservoirs = caseCollection->reservoirs().childrenByType();
    if ( !reservoirs.empty() )
    {
        auto caseDescription = reservoirs.front()->caseUserDescription();
        newStatisticsCase->setWellDataSourceCase( caseDescription );
    }

    newStatisticsCase->openEclipseGridFile();
    newStatisticsCase->computeActiveCellsBoundingBox();
    newStatisticsCase->selectAllTimeSteps();

    return newStatisticsCase;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIdenticalGridCaseGroup::contains( RimEclipseCase* reservoir ) const
{
    CVF_ASSERT( reservoir );

    for ( size_t i = 0; i < caseCollection()->reservoirs().size(); i++ )
    {
        RimEclipseCase* rimReservoir = caseCollection()->reservoirs()[i];
        if ( reservoir->gridFileName() == rimReservoir->gridFileName() )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RimIdenticalGridCaseGroup::unionOfActiveCells( RiaDefines::PorosityModelType porosityType )
{
    if ( porosityType == RiaDefines::PorosityModelType::MATRIX_MODEL )
    {
        return m_unionOfMatrixActiveCells.p();
    }
    else
    {
        return m_unionOfFractureActiveCells.p();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RimIdenticalGridCaseGroup::isStatisticsCaseCollection( RimCaseCollection* rimCaseCollection )
{
    caf::PdmFieldHandle* parentField = rimCaseCollection->parentField();
    if ( parentField )
    {
        if ( parentField->keyword() == "StatisticsCaseCollection" )
        {
            return true;
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RimIdenticalGridCaseGroup::mainCase()
{
    if ( !caseCollection()->reservoirs().empty() )
    {
        return caseCollection()->reservoirs()[0];
    }
    else
    {
        return nullptr;
    }
}
