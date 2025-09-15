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

#include "RifReaderEclipseOutput.h"

#include "RiaCellDividingTools.h"
#include "RiaEclipseUnitTools.h"
#include "RiaLogging.h"

#include "RiaStringEncodingTools.h"
#include "RifActiveCellsReader.h"
#include "RifEclipseInputFileTools.h"
#include "RifEclipseOutputFileTools.h"
#include "RifEclipseRestartDataAccess.h"
#include "RifEdfmTools.h"
#include "RifHdf5ReaderInterface.h"
#include "RifOpmRadialGridTools.h"
#include "RifReaderEclipseWell.h"

#ifdef USE_HDF5
#include "RifHdf5Reader.h"
#endif

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigEquil.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"
#include "Well/RigWellResultPoint.h"

#include "cafProgressInfo.h"

#include "cvfTrace.h"

#include "ert/ecl/ecl_kw_magic.h"
#include "ert/ecl/ecl_nnc_data.h"
#include "ert/ecl/ecl_nnc_export.h"
#include "ert/ecl/ecl_nnc_geometry.h"

#include <QDateTime>
#include <QFileInfo>

#include <cmath> // Needed for HUGE_VAL on Linux
#include <iostream>
#include <map>

#ifdef USE_OPENMP
#include <omp.h>
#endif

//--------------------------------------------------------------------------------------------------
///     ECLIPSE cell numbering layout:
///        Lower layer:   Upper layer
///        Low Depth      High Depth
///        Low K          High K
///        Shallow        Deep
///       2---3           6---7
///       |   |           |   |
///       0---1           4---5
///
///
///
//--------------------------------------------------------------------------------------------------

// The indexing conventions for vertices in ECLIPSE
//
//      2-------------3
//     /|            /|
//    / |           / |               /j
//   /  |          /  |              /
//  0-------------1   |             *---i
//  |   |         |   |             |
//  |   6---------|---7             |
//  |  /          |  /              |k
//  | /           | /
//  |/            |/
//  4-------------5
//  vertex indices
//
// The indexing conventions for vertices in ResInsight
//
//      7-------------6             |k
//     /|            /|             | /j
//    / |           / |             |/
//   /  |          /  |             *---i
//  4-------------5   |
//  |   |         |   |
//  |   3---------|---2
//  |  /          |  /
//  | /           | /
//  |/            |/
//  0-------------1
//  vertex indices
//

static const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseOutput::RifReaderEclipseOutput()
{
    m_fileName.clear();
    m_filesWithSameBaseName.clear();

    m_eclipseCaseData = nullptr;

    m_ecl_init_file        = nullptr;
    m_dynamicResultsAccess = nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Destructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseOutput::~RifReaderEclipseOutput()
{
    if ( m_ecl_init_file )
    {
        ecl_file_close( m_ecl_init_file );
    }
    m_ecl_init_file = nullptr;

    if ( m_dynamicResultsAccess.notNull() )
    {
        m_dynamicResultsAccess->close();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::transferGridCellData( RigMainGrid*         mainGrid,
                                                   RigActiveCellInfo*   activeCellInfo,
                                                   RigActiveCellInfo*   fractureActiveCellInfo,
                                                   RigGridBase*         localGrid,
                                                   const ecl_grid_type* localEclGrid,
                                                   size_t               matrixActiveStartIndex,
                                                   size_t               fractureActiveStartIndex,
                                                   bool                 invalidateLongPyramidCells )
{
    CVF_ASSERT( activeCellInfo && fractureActiveCellInfo );

    int    cellCount      = ecl_grid_get_global_size( localEclGrid );
    size_t cellStartIndex = mainGrid->reservoirCells().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid( localGrid );
    mainGrid->reservoirCells().resize( cellStartIndex + cellCount, defaultCell );

    mainGrid->nodes().resize( nodeStartIndex + cellCount * 8, cvf::Vec3d( 0, 0, 0 ) );

    // Loop over cells and fill them with data

#pragma omp parallel for
    for ( int gridLocalCellIndex = 0; gridLocalCellIndex < cellCount; ++gridLocalCellIndex )
    {
        RigCell& cell = mainGrid->cell( cellStartIndex + gridLocalCellIndex );

        cell.setGridLocalCellIndex( gridLocalCellIndex );

        // Active cell index
        int matrixActiveIndex = ecl_grid_get_active_index1( localEclGrid, gridLocalCellIndex );
        if ( matrixActiveIndex != -1 )
        {
            activeCellInfo->setCellResultIndex( cellStartIndex + gridLocalCellIndex, matrixActiveStartIndex + matrixActiveIndex );
        }

        int fractureActiveIndex = ecl_grid_get_active_fracture_index1( localEclGrid, gridLocalCellIndex );
        if ( fractureActiveIndex != -1 )
        {
            fractureActiveCellInfo->setCellResultIndex( cellStartIndex + gridLocalCellIndex, fractureActiveStartIndex + fractureActiveIndex );
        }

        // Parent cell index
        int parentCellIndex = ecl_grid_get_parent_cell1( localEclGrid, gridLocalCellIndex );
        if ( parentCellIndex == -1 )
        {
            cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );
        }
        else
        {
            cell.setParentCellIndex( parentCellIndex );
        }

        // Corner coordinates
        int cIdx;
        for ( cIdx = 0; cIdx < 8; ++cIdx )
        {
            double* point = mainGrid->nodes()[nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cellMappingECLRi[cIdx]].ptr();
            ecl_grid_get_cell_corner_xyz1( localEclGrid, gridLocalCellIndex, cIdx, &( point[0] ), &( point[1] ), &( point[2] ) );
            point[2]                   = -point[2]; // Flipping Z making depth become negative z values
            cell.cornerIndices()[cIdx] = nodeStartIndex + (size_t)gridLocalCellIndex * 8 + cIdx;
        }

        // Sub grid in cell
        const ecl_grid_type* subGrid = ecl_grid_get_cell_lgr1( localEclGrid, gridLocalCellIndex );
        if ( subGrid != nullptr )
        {
            int subGridId = ecl_grid_get_lgr_nr( subGrid );
            CVF_ASSERT( subGridId > 0 );
            cell.setSubGrid( static_cast<RigLocalGrid*>( mainGrid->gridById( subGridId ) ) );
        }

        // Mark inactive long pyramid looking cells as invalid
        // Forslag
        // if (!invalid && (cell.isInCoarseCell() || (!cell.isActiveInMatrixModel() &&
        // !cell.isActiveInFractureModel()) ) )
        if ( invalidateLongPyramidCells )
        {
            cell.setInvalid( cell.isLongPyramidCell() );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Read geometry from file given by name into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::transferGeometry( const ecl_grid_type* mainEclGrid, RigEclipseCaseData* eclipseCase, bool invalidateLongThinCells )
{
    CVF_ASSERT( eclipseCase );

    if ( !mainEclGrid )
    {
        // Some error
        return false;
    }

    RigActiveCellInfo* activeCellInfo         = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveCellInfo = eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    CVF_ASSERT( activeCellInfo && fractureActiveCellInfo );

    RigMainGrid* mainGrid = eclipseCase->mainGrid();
    CVF_ASSERT( mainGrid );
    mainGrid->setCellCounts( cvf::Vec3st( ecl_grid_get_nx( mainEclGrid ), ecl_grid_get_ny( mainEclGrid ), ecl_grid_get_nz( mainEclGrid ) ) );

    // std::string mainGridName = ecl_grid_get_name(mainEclGrid);
    // ERT returns file path to grid file as name for main grid
    mainGrid->setGridName( "Main grid" );

    mainGrid->setDualPorosity( ecl_grid_dual_grid( mainEclGrid ) );

    // Get and set grid and lgr metadata

    size_t totalCellCount = static_cast<size_t>( ecl_grid_get_global_size( mainEclGrid ) );

    int numLGRs = ecl_grid_get_num_lgr( mainEclGrid );
    int lgrIdx;
    for ( lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx )
    {
        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr( mainEclGrid, lgrIdx );

        std::string lgrName = ecl_grid_get_name( localEclGrid );
        int         lgrId   = ecl_grid_get_lgr_nr( localEclGrid );

        RigLocalGrid* localGrid = new RigLocalGrid( mainGrid );
        localGrid->setGridId( lgrId );
        mainGrid->addLocalGrid( localGrid );

        localGrid->setIndexToStartOfCells( totalCellCount );
        localGrid->setGridName( lgrName );

        cvf::Vec3st cellCounts( ecl_grid_get_nx( localEclGrid ), ecl_grid_get_ny( localEclGrid ), ecl_grid_get_nz( localEclGrid ) );
        localGrid->setCellCounts( cellCounts );

        totalCellCount += ecl_grid_get_global_size( localEclGrid );
    }

    activeCellInfo->setReservoirCellCount( totalCellCount );
    fractureActiveCellInfo->setReservoirCellCount( totalCellCount );

    // Reserve room for the cells and nodes and fill them with data

    mainGrid->reservoirCells().reserve( totalCellCount );
    mainGrid->nodes().reserve( 8 * totalCellCount );

    caf::ProgressInfo progInfo( 3 + numLGRs, "" );

    {
        auto task = progInfo.task( "Loading Main Grid Data", 3 );
        transferGridCellData( mainGrid, activeCellInfo, fractureActiveCellInfo, mainGrid, mainEclGrid, 0, 0, invalidateLongThinCells );
    }

    size_t globalMatrixActiveSize   = ecl_grid_get_nactive( mainEclGrid );
    size_t globalFractureActiveSize = ecl_grid_get_nactive_fracture( mainEclGrid );

    activeCellInfo->setGridCount( 1 + numLGRs );
    fractureActiveCellInfo->setGridCount( 1 + numLGRs );

    activeCellInfo->setGridActiveCellCounts( 0, globalMatrixActiveSize );
    fractureActiveCellInfo->setGridActiveCellCounts( 0, globalFractureActiveSize );

    transferCoarseningInfo( mainEclGrid, mainGrid );

    for ( lgrIdx = 0; lgrIdx < numLGRs; ++lgrIdx )
    {
        auto task = progInfo.task( "LGR number " + QString::number( lgrIdx + 1 ), 1 );

        ecl_grid_type* localEclGrid = ecl_grid_iget_lgr( mainEclGrid, lgrIdx );
        RigLocalGrid*  localGrid    = static_cast<RigLocalGrid*>( mainGrid->gridByIndex( lgrIdx + 1 ) );

        transferGridCellData( mainGrid,
                              activeCellInfo,
                              fractureActiveCellInfo,
                              localGrid,
                              localEclGrid,
                              globalMatrixActiveSize,
                              globalFractureActiveSize,
                              invalidateLongThinCells );

        int matrixActiveCellCount = ecl_grid_get_nactive( localEclGrid );
        globalMatrixActiveSize += matrixActiveCellCount;

        int fractureActiveCellCount = ecl_grid_get_nactive_fracture( localEclGrid );
        globalFractureActiveSize += fractureActiveCellCount;

        activeCellInfo->setGridActiveCellCounts( lgrIdx + 1, matrixActiveCellCount );
        fractureActiveCellInfo->setGridActiveCellCounts( lgrIdx + 1, fractureActiveCellCount );

        transferCoarseningInfo( localEclGrid, localGrid );
    }

    mainGrid->initAllSubGridsParentGridPointer();
    activeCellInfo->computeDerivedData();
    fractureActiveCellInfo->computeDerivedData();

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Open file and read geometry into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::open( const QString& fileName, RigEclipseCaseData* eclipseCaseData )
{
    CVF_ASSERT( eclipseCaseData );
    caf::ProgressInfo progress( 100, "Reading Grid" );

    if ( !RifEclipseOutputFileTools::isValidEclipseFileName( fileName ) )
    {
        QString errorMessage = QFileInfo( fileName ).fileName() +
                               QString( " is not a valid Eclipse file name.\n"
                                        "Please make sure the file does not contain a mix of upper and lower case letters." );
        RiaLogging::error( errorMessage );
        return false;
    }

    QStringList fileSet;
    {
        auto task = progress.task( "Get set of files" );

        if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &fileSet ) ) return false;

        m_fileName              = fileName;
        m_filesWithSameBaseName = fileSet;
    }

    ecl_grid_type* mainEclGrid = nullptr;
    {
        auto task = progress.task( "Open Init File and Load Main Grid", 19 );

        openInitFile();

        // Read geometry
        // Todo: Needs to check existence of file before calling ert, else it will abort
        mainEclGrid = loadAllGrids();
        if ( !mainEclGrid )
        {
            QString errorMessage = QString( " Failed to create a main grid from file\n%1" ).arg( m_fileName );
            RiaLogging::error( errorMessage );

            return false;
        }
    }

    {
        auto task = progress.task( "Transferring grid geometry", 10 );
        if ( !transferGeometry( mainEclGrid, eclipseCaseData, invalidateLongThinCells() ) ) return false;

        RifOpmRadialGridTools::importCoordinatesForRadialGrid( fileName.toStdString(), eclipseCaseData->mainGrid() );
    }

    auto iLimitFromEdfm = RifEdfmTools::checkForEdfmLimitI( fileName );
    if ( iLimitFromEdfm > 0 )
    {
        eclipseCaseData->mainGrid()->invalidateCellsAboveI( iLimitFromEdfm - 1 /* zero based index */ );
    }

    {
        auto task = progress.task( "Reading faults", 10 );

        if ( isFaultImportEnabled() )
        {
            cvf::Collection<RigFault> faults;

            importFaults( fileSet, &faults );

            RigMainGrid* mainGrid = eclipseCaseData->mainGrid();
            mainGrid->setFaults( faults );
        }
    }

    m_eclipseCaseData = eclipseCaseData;

    {
        auto task = progress.task( "Reading Results Meta data", 25 );
        buildMetaData( mainEclGrid );
    }

    {
        auto task = progress.task( "Handling NCC data", 20 );
        if ( isNNCsEnabled() )
        {
            caf::ProgressInfo nncProgress( 10, "" );
            RigMainGrid*      mainGrid = eclipseCaseData->mainGrid();

            {
                auto subNncTask = nncProgress.task( "Reading static NNC data" );
                transferStaticNNCData( mainEclGrid, m_ecl_init_file, mainGrid );
            }

            // This test should probably be improved to test more directly for presence of NNC data
            if ( m_eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasFlowDiagUsableFluxes() )
            {
                auto subNncTask = nncProgress.task( "Reading dynamic NNC data" );
                transferDynamicNNCData( mainEclGrid, mainGrid );
            }

            RigActiveCellInfo* activeCellInfo = m_eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

            bool includeInactiveCells = includeInactiveCellsInFaultGeometry();
            mainGrid->nncData()->setSourceDataForProcessing( mainGrid, activeCellInfo, includeInactiveCells );
        }
    }

    {
        auto task = progress.task( "Handling well information", 10 );
        if ( loadWellDataEnabled() )
        {
            std::vector<QDateTime>              filteredTimeSteps;
            std::vector<RigEclipseTimeStepInfo> filteredTimeStepInfos = createFilteredTimeStepInfos();
            for ( auto a : filteredTimeStepInfos )
            {
                filteredTimeSteps.push_back( a.m_date );
            }

            std::vector<std::string> gridNames;
            gridNames.push_back( "global" );
            for ( int i = 0; i < ecl_grid_get_num_lgr( mainEclGrid ); i++ )
            {
                const char* gridName = ecl_grid_iget_lgr_name( mainEclGrid, i );
                gridNames.push_back( gridName );
            }

            RifReaderEclipseWell::readWellCells( m_dynamicResultsAccess.p(),
                                                 m_eclipseCaseData,
                                                 filteredTimeSteps,
                                                 gridNames,
                                                 isImportOfCompleteMswDataEnabled() );
        }
        else
        {
            RiaLogging::info( "Skipping import of simulation well data" );
        }
    }

    {
        auto task = progress.task( "Releasing reader memory", 5 );
        ecl_grid_free( mainEclGrid );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::setHdf5FileName( const QString& fileName )
{
    CVF_ASSERT( m_eclipseCaseData );

    RigCaseCellResultsData* matrixModelResults = m_eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    CVF_ASSERT( matrixModelResults );

    if ( fileName.isEmpty() )
    {
        RiaLogging::info( "HDF: Removing all existing Sour Sim data ..." );
        matrixModelResults->eraseAllSourSimData();

        return;
    }

    RiaLogging::info( QString( "HDF: Start import of data from : " ).arg( fileName ) );

    RiaLogging::info( "HDF: Removing all existing Sour Sim data ..." );
    matrixModelResults->eraseAllSourSimData();

    std::vector<RigEclipseTimeStepInfo> timeStepInfos = createFilteredTimeStepInfos();

    std::unique_ptr<RifHdf5ReaderInterface> hdf5ReaderInterface;
#ifdef USE_HDF5
    hdf5ReaderInterface = std::unique_ptr<RifHdf5ReaderInterface>( new RifHdf5Reader( fileName ) );
#endif // USE_HDF5

    if ( !hdf5ReaderInterface )
    {
        return;
    }

    std::vector<QDateTime> sourSimTimeSteps = hdf5ReaderInterface->timeSteps();
    if ( sourSimTimeSteps.empty() )
    {
        RiaLogging::error( "HDF: No data available from SourSim" );

        return;
    }

    if ( !timeStepInfos.empty() )
    {
        if ( allTimeSteps().size() != sourSimTimeSteps.size() )
        {
            RiaLogging::error(
                QString( "HDF: Time step count mismatch, Eclipse : %1 ; HDF : %2 " ).arg( allTimeSteps().size() ).arg( sourSimTimeSteps.size() ) );

            return;
        }

        bool isTimeStampsEqual = true;
        for ( size_t i = 0; i < timeStepInfos.size(); i++ )
        {
            size_t indexOnFile = timeStepIndexOnFile( i );
            if ( indexOnFile < sourSimTimeSteps.size() )
            {
                if ( !isEclipseAndSoursimTimeStepsEqual( timeStepInfos[i].m_date, sourSimTimeSteps[indexOnFile] ) )
                {
                    isTimeStampsEqual = false;
                }
            }
            else
            {
                RiaLogging::error(
                    QString( "HDF: Time step count mismatch, Eclipse : %1 ; HDF : %2 " ).arg( timeStepInfos.size() ).arg( sourSimTimeSteps.size() ) );

                // We have less soursim time steps than eclipse time steps
                isTimeStampsEqual = false;
            }
        }

        if ( !isTimeStampsEqual ) return;
    }
    else
    {
        // Use time steps from HDF to define the time steps
        QDateTime firstDate = sourSimTimeSteps[0];

        std::vector<double> daysSinceSimulationStart;

        for ( auto d : sourSimTimeSteps )
        {
            daysSinceSimulationStart.push_back( firstDate.daysTo( d ) );
        }

        std::vector<int> reportNumbers;
        if ( m_dynamicResultsAccess.notNull() )
        {
            reportNumbers = m_dynamicResultsAccess->reportNumbers();
        }
        else
        {
            for ( size_t i = 0; i < sourSimTimeSteps.size(); i++ )
            {
                reportNumbers.push_back( static_cast<int>( i ) );
            }
        }

        timeStepInfos = RigEclipseTimeStepInfo::createTimeStepInfos( sourSimTimeSteps, reportNumbers, daysSinceSimulationStart );
    }

    QStringList resultNames = hdf5ReaderInterface->propertyNames();

    for ( int i = 0; i < resultNames.size(); ++i )
    {
        RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::SOURSIMRL, resultNames[i] );
        matrixModelResults->createResultEntry( resAddr, false );
        matrixModelResults->setTimeStepInfos( resAddr, timeStepInfos );
    }

    m_hdfReaderInterface = std::move( hdf5ReaderInterface );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::setFileDataAccess( RifEclipseRestartDataAccess* restartDataAccess )
{
    m_dynamicResultsAccess = restartDataAccess;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const size_t* RifReaderEclipseOutput::eclipseCellIndexMapping()
{
    return cellMappingECLRi;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::importEquilData( const QString&      deckFileName,
                                              const QString&      includeStatementAbsolutePathPrefix,
                                              RigEclipseCaseData* eclipseCase )
{
    QFile file( deckFileName );
    if ( file.open( QFile::ReadOnly ) )
    {
        const QString keyword( "EQUIL" );
        const QString keywordToStopParsing( "SCHEDULE" );
        auto          keywordContent = RifEclipseInputFileTools::readKeywordContentFromFile( keyword, keywordToStopParsing, file );

        std::vector<RigEquil> equilItems;
        for ( const auto& s : keywordContent )
        {
            RigEquil equilRec = RigEquil::parseString( s );

            equilItems.push_back( equilRec );
        }

        eclipseCase->setEquilData( equilItems );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::transferStaticNNCData( const ecl_grid_type* mainEclGrid, ecl_file_type* init_file, RigMainGrid* mainGrid )
{
    if ( !m_ecl_init_file ) return;

    CVF_ASSERT( mainEclGrid && mainGrid );

    // Get the data from ERT
    ecl_nnc_geometry_type* nnc_geo = ecl_nnc_geometry_alloc( mainEclGrid );
    if ( nnc_geo )
    {
        ecl_nnc_data_type* tran_data = ecl_nnc_data_alloc_tran( mainEclGrid, nnc_geo, ecl_file_get_global_view( init_file ) );
        if ( tran_data )
        {
            int numNNC       = ecl_nnc_data_get_size( tran_data );
            int geometrySize = ecl_nnc_geometry_size( nnc_geo );
            CVF_ASSERT( numNNC == geometrySize );

            if ( numNNC > 0 )
            {
                // Transform to our own data structures
                RigConnectionContainer nncConnections;
                std::vector<double>    transmissibilityValuesTemp;

                const double* transValues = ecl_nnc_data_get_values( tran_data );

                for ( int nIdx = 0; nIdx < numNNC; ++nIdx )
                {
                    const ecl_nnc_pair_type* geometry_pair = ecl_nnc_geometry_iget( nnc_geo, nIdx );
                    RigGridBase*             grid1         = mainGrid->gridByIndex( geometry_pair->grid_nr1 );
                    RigGridBase*             grid2         = mainGrid->gridByIndex( geometry_pair->grid_nr2 );

                    RigConnection nncConnection( grid1->reservoirCellIndex( geometry_pair->global_index1 ),
                                                 grid2->reservoirCellIndex( geometry_pair->global_index2 ) );

                    nncConnections.push_back( nncConnection );

                    transmissibilityValuesTemp.push_back( transValues[nIdx] );
                }

                mainGrid->nncData()->setEclipseConnections( nncConnections );
                mainGrid->nncData()->makeScalarResultAndSetValues( RiaDefines::propertyNameCombTrans(), transmissibilityValuesTemp );
            }

            ecl_nnc_data_free( tran_data );
        }

        ecl_nnc_geometry_free( nnc_geo );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::transferDynamicNNCData( const ecl_grid_type* mainEclGrid, RigMainGrid* mainGrid )
{
    CVF_ASSERT( mainEclGrid && mainGrid );

    if ( m_dynamicResultsAccess.isNull() ) return;

    size_t timeStepCount = m_dynamicResultsAccess->timeStepCount();

    std::vector<std::vector<double>>& waterFluxData =
        mainGrid->nncData()->makeDynamicConnectionScalarResult( RiaDefines::propertyNameFluxWat(), timeStepCount );
    std::vector<std::vector<double>>& oilFluxData =
        mainGrid->nncData()->makeDynamicConnectionScalarResult( RiaDefines::propertyNameFluxOil(), timeStepCount );
    std::vector<std::vector<double>>& gasFluxData =
        mainGrid->nncData()->makeDynamicConnectionScalarResult( RiaDefines::propertyNameFluxGas(), timeStepCount );

    for ( size_t timeStep = 0; timeStep < timeStepCount; ++timeStep )
    {
        m_dynamicResultsAccess->dynamicNNCResults( mainEclGrid, timeStep, &waterFluxData[timeStep], &oilFluxData[timeStep], &gasFluxData[timeStep] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::openAndReadActiveCellData( const QString&                fileName,
                                                        const std::vector<QDateTime>& mainCaseTimeSteps,
                                                        RigEclipseCaseData*           eclipseCase )
{
    CVF_ASSERT( eclipseCase );

    // It is required to have a main grid before reading active cell data
    if ( !eclipseCase->mainGrid() )
    {
        return false;
    }

    // Get set of files
    QStringList fileSet;
    if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &fileSet ) ) return false;

    // Keep the set of files of interest
    m_filesWithSameBaseName = fileSet;
    m_eclipseCaseData       = eclipseCase;
    m_fileName              = fileName;

    if ( !readActiveCellInfo() )
    {
        return false;
    }

    ensureDynamicResultAccessIsPresent();
    if ( m_dynamicResultsAccess.notNull() )
    {
        m_dynamicResultsAccess->setTimeSteps( mainCaseTimeSteps );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::readActiveCellInfo()
{
    CVF_ASSERT( m_eclipseCaseData );
    CVF_ASSERT( m_eclipseCaseData->mainGrid() );

    std::vector<std::vector<int>> actnumValuesPerGrid;

    {
        // If INIT file is present and PORV is found, use PORV as basis for active cells
        QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType( m_filesWithSameBaseName, ECL_INIT_FILE );
        if ( initFileName.size() > 0 )
        {
            ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( initFileName ).data(), ECL_FILE_CLOSE_STREAM );
            if ( ecl_file )
            {
                bool isDualPorosity    = m_eclipseCaseData->mainGrid()->isDualPorosity();
                int  cellCountMainGrid = static_cast<int>( m_eclipseCaseData->mainGrid()->cellCount() );
                actnumValuesPerGrid    = RifActiveCellsReader::activeCellsFromPorvKeyword( ecl_file, isDualPorosity, cellCountMainGrid );
                ecl_file_close( ecl_file );
            }
        }
    }

    if ( actnumValuesPerGrid.empty() )
    {
        // Try ACTNUM from grid file as basis for active cells
        QString egridFileName = RifEclipseOutputFileTools::firstFileNameOfType( m_filesWithSameBaseName, ECL_EGRID_FILE );
        if ( egridFileName.size() > 0 )
        {
            ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( egridFileName ).data(), ECL_FILE_CLOSE_STREAM );
            if ( ecl_file )
            {
                actnumValuesPerGrid = RifActiveCellsReader::activeCellsFromActnumKeyword( ecl_file );
                ecl_file_close( ecl_file );
            }
        }
    }

    return RifEclipseOutputFileTools::assignActiveCellData( actnumValuesPerGrid, m_eclipseCaseData );
}

//--------------------------------------------------------------------------------------------------
/// Build meta data - get states and results info
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::buildMetaData( ecl_grid_type* grid )
{
    CVF_ASSERT( m_eclipseCaseData );
    CVF_ASSERT( !m_filesWithSameBaseName.empty() );

    caf::ProgressInfo progInfo( m_filesWithSameBaseName.size() + 3, "" );

    progInfo.setNextProgressIncrement( m_filesWithSameBaseName.size() );

    std::vector<RigEclipseTimeStepInfo> timeStepInfos;

    // Create access object for dynamic results
    ensureDynamicResultAccessIsPresent();
    if ( m_dynamicResultsAccess.notNull() )
    {
        progInfo.incrementProgress();

        m_dynamicResultsAccess->open();

        timeStepInfos    = createFilteredTimeStepInfos();
        auto keywordInfo = m_dynamicResultsAccess->keywordValueCounts();

        RifEclipseOutputFileTools::createResultEntries( keywordInfo,
                                                        timeStepInfos,
                                                        RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                        m_eclipseCaseData,
                                                        m_dynamicResultsAccess->timeStepCount() );
    }

    progInfo.incrementProgress();

    openInitFile();

    // Unit system
    {
        // Default units type is METRIC
        RiaDefines::EclipseUnitSystem unitsType = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
        int                           unitsTypeValue;

        if ( m_dynamicResultsAccess.notNull() )
        {
            unitsTypeValue = m_dynamicResultsAccess->readUnitsType();
        }
        else
        {
            if ( m_ecl_init_file )
            {
                unitsTypeValue = RifEclipseOutputFileTools::readUnitsType( m_ecl_init_file );
            }
            else
            {
                unitsTypeValue = ecl_grid_get_unit_system( grid );
            }
        }

        if ( unitsTypeValue == 2 )
        {
            unitsType = RiaDefines::EclipseUnitSystem::UNITS_FIELD;
        }
        else if ( unitsTypeValue == 3 )
        {
            unitsType = RiaDefines::EclipseUnitSystem::UNITS_LAB;
        }
        m_eclipseCaseData->setUnitsType( unitsType );
    }

    progInfo.incrementProgress();

    if ( m_ecl_init_file )
    {
        std::vector<ecl_file_type*> filesUsedToFindAvailableKeywords;
        filesUsedToFindAvailableKeywords.push_back( m_ecl_init_file );

        auto keywordInfo = RifEclipseOutputFileTools::keywordValueCounts( filesUsedToFindAvailableKeywords );

        std::vector<RigEclipseTimeStepInfo> staticTimeStepInfo;
        if ( !timeStepInfos.empty() )
        {
            staticTimeStepInfo.push_back( timeStepInfos.front() );
        }

        RifEclipseOutputFileTools::createResultEntries( keywordInfo,
                                                        staticTimeStepInfo,
                                                        RiaDefines::ResultCatType::STATIC_NATIVE,
                                                        m_eclipseCaseData,
                                                        1 );
    }
}

//--------------------------------------------------------------------------------------------------
/// Create results access object (.UNRST or .X0001 ... .XNNNN)
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::ensureDynamicResultAccessIsPresent()
{
    if ( m_dynamicResultsAccess.isNull() )
    {
        m_dynamicResultsAccess = RifEclipseOutputFileTools::createDynamicResultAccess( m_fileName );
    }
}

//--------------------------------------------------------------------------------------------------
/// Get all values of a given static result as doubles
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    CVF_ASSERT( values );

    if ( result.compare( "ACTNUM", Qt::CaseInsensitive ) == 0 )
    {
        RigActiveCellInfo* activeCellInfo = m_eclipseCaseData->activeCellInfo( matrixOrFracture );
        values->resize( activeCellInfo->reservoirActiveCellCount(), 1.0 );

        return true;
    }

    openInitFile();

    if ( m_ecl_init_file )
    {
        std::vector<double> fileValues;

        size_t numOccurrences = ecl_file_get_num_named_kw( m_ecl_init_file, result.toLatin1().data() );
        size_t i;
        for ( i = 0; i < numOccurrences; i++ )
        {
            std::vector<double> partValues;
            RifEclipseOutputFileTools::keywordData( m_ecl_init_file, result, i, &partValues );
            fileValues.insert( fileValues.end(), partValues.begin(), partValues.end() );
        }

        RifEclipseOutputFileTools::extractResultValuesBasedOnPorosityModel( m_eclipseCaseData, matrixOrFracture, values, fileValues );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::sourSimRlResult( const QString& result, size_t stepIndex, std::vector<double>* values )
{
    values->clear();

    if ( !m_hdfReaderInterface ) return;

    if ( m_eclipseCaseData->mainGrid()->gridCount() == 0 )
    {
        RiaLogging::error( "No grids available" );

        return;
    }

    RigActiveCellInfo* fracActCellInfo = m_eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    size_t             activeCellCount = fracActCellInfo->gridActiveCellCounts( 0 );

    size_t fileIndex = timeStepIndexOnFile( stepIndex );

    m_hdfReaderInterface->dynamicResult( result, fileIndex, values );

    if ( activeCellCount != values->size() )
    {
        values->clear();

        RiaLogging::error( "SourSimRL results does not match the number of active cells in the grid" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifReaderEclipseOutput::allTimeSteps() const
{
    std::vector<QDateTime> steps;
    if ( m_dynamicResultsAccess.notNull() )
    {
        std::vector<double> dymmy;
        m_dynamicResultsAccess->timeSteps( &steps, &dymmy );
    }

    return steps;
}

//--------------------------------------------------------------------------------------------------
/// Get dynamic result at given step index. Will concatenate values for the main grid and all sub grids.
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::dynamicResult( const QString&                result,
                                            RiaDefines::PorosityModelType matrixOrFracture,
                                            size_t                        stepIndex,
                                            std::vector<double>*          values )
{
    ensureDynamicResultAccessIsPresent();

    if ( m_dynamicResultsAccess.notNull() )
    {
        size_t indexOnFile = timeStepIndexOnFile( stepIndex );

        std::vector<double> fileValues;
        if ( !m_dynamicResultsAccess->results( result, indexOnFile, m_eclipseCaseData->mainGrid()->gridCountOnFile(), &fileValues ) )
        {
            return false;
        }

        RifEclipseOutputFileTools::extractResultValuesBasedOnPorosityModel( m_eclipseCaseData, matrixOrFracture, values, fileValues );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseTimeStepInfo> RifReaderEclipseOutput::createFilteredTimeStepInfos()
{
    std::vector<RigEclipseTimeStepInfo> timeStepInfos;

    if ( m_dynamicResultsAccess.notNull() )
    {
        std::vector<QDateTime> timeStepsOnFile;
        std::vector<double>    daysSinceSimulationStartOnFile;
        std::vector<int>       reportNumbersOnFile;

        m_dynamicResultsAccess->timeSteps( &timeStepsOnFile, &daysSinceSimulationStartOnFile );
        reportNumbersOnFile = m_dynamicResultsAccess->reportNumbers();

        if ( timeStepsOnFile.size() != daysSinceSimulationStartOnFile.size() ) return timeStepInfos;
        if ( timeStepsOnFile.size() != reportNumbersOnFile.size() ) return timeStepInfos;

        for ( size_t i = 0; i < timeStepsOnFile.size(); i++ )
        {
            if ( isTimeStepIncludedByFilter( i ) )
            {
                timeStepInfos.push_back(
                    RigEclipseTimeStepInfo( timeStepsOnFile[i], reportNumbersOnFile[i], daysSinceSimulationStartOnFile[i] ) );
            }
        }
    }

    return timeStepInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::isEclipseAndSoursimTimeStepsEqual( const QDateTime& eclipseDateTime, const QDateTime& sourSimDateTime )
{
    // Compare date down to and including seconds
    // Compare of complete date time objects will often result in differences

    const int     secondsThreshold = 4;
    const QString dateStr( "yyyy.MMM.dd hh:mm:ss:zzz" );

    int secondsDiff = eclipseDateTime.secsTo( sourSimDateTime );
    if ( secondsDiff > secondsThreshold )
    {
        RiaLogging::error( "HDF: Time steps does not match" );

        RiaLogging::error( QString( "  %1 - Eclipse" ).arg( eclipseDateTime.toString( dateStr ) ) );
        RiaLogging::error( QString( "  %1 - SourSim" ).arg( sourSimDateTime.toString( dateStr ) ) );

        return false;
    }

    if ( eclipseDateTime.time().second() != sourSimDateTime.time().second() )
    {
        RiaLogging::warning( "HDF: Time steps differ, but within time step compare threshold" );
        RiaLogging::warning( QString( "  %1 - Eclipse" ).arg( eclipseDateTime.toString( dateStr ) ) );
        RiaLogging::warning( QString( "  %1 - SourSim" ).arg( sourSimDateTime.toString( dateStr ) ) );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
ecl_grid_type* RifReaderEclipseOutput::loadAllGrids() const
{
    ecl_grid_type* mainEclGrid = ecl_grid_alloc( RiaStringEncodingTools::toNativeEncoded( m_fileName ).data() );

    if ( m_ecl_init_file )
    {
        // TODO : ecl_grid_alloc() will automatically read ACTNUM from EGRID file, and reading of active cell
        // information can be skipped if PORV is available

        bool isDualPorosity    = ecl_grid_dual_grid( mainEclGrid );
        auto cellCountMainGrid = ecl_grid_get_global_size( mainEclGrid );
        auto activeCells       = RifActiveCellsReader::activeCellsFromPorvKeyword( m_ecl_init_file, isDualPorosity, cellCountMainGrid );

        if ( !activeCells.empty() )
        {
            RifActiveCellsReader::applyActiveCellsToAllGrids( mainEclGrid, activeCells );
        }
    }

    return mainEclGrid;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::updateFromGridCount( size_t gridCount )
{
    if ( m_dynamicResultsAccess.notNull() )
    {
        m_dynamicResultsAccess->updateFromGridCount( gridCount );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::openInitFile()
{
    if ( m_ecl_init_file )
    {
        return;
    }

    QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType( m_filesWithSameBaseName, ECL_INIT_FILE );
    if ( initFileName.size() > 0 )
    {
        m_ecl_init_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( initFileName ).data(), ECL_FILE_CLOSE_STREAM );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::transferCoarseningInfo( const ecl_grid_type* eclGrid, RigGridBase* grid )
{
    int coarseGroupCount = ecl_grid_get_num_coarse_groups( eclGrid );
    for ( int i = 0; i < coarseGroupCount; i++ )
    {
        ecl_coarse_cell_type* coarse_cell = ecl_grid_iget_coarse_group( eclGrid, i );

        if ( coarse_cell )
        {
            size_t i1 = static_cast<size_t>( ecl_coarse_cell_get_i1( coarse_cell ) );
            size_t i2 = static_cast<size_t>( ecl_coarse_cell_get_i2( coarse_cell ) );
            size_t j1 = static_cast<size_t>( ecl_coarse_cell_get_j1( coarse_cell ) );
            size_t j2 = static_cast<size_t>( ecl_coarse_cell_get_j2( coarse_cell ) );
            size_t k1 = static_cast<size_t>( ecl_coarse_cell_get_k1( coarse_cell ) );
            size_t k2 = static_cast<size_t>( ecl_coarse_cell_get_k2( coarse_cell ) );

            grid->addCoarseningBox( i1, i2, j1, j2, k1, k2 );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> RifReaderEclipseOutput::availablePhases() const
{
    if ( m_dynamicResultsAccess.notNull() )
    {
        return m_dynamicResultsAccess->availablePhases();
    }

    return std::set<RiaDefines::PhaseType>();
}
