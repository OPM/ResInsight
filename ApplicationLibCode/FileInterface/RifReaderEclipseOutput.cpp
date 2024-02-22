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
#include "RifHdf5ReaderInterface.h"
#include "RifOpmGridTools.h"
#include "RifReaderSettings.h"

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
#include "RigSimWellData.h"
#include "RigWellResultFrame.h"
#include "RigWellResultPoint.h"

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

//**************************************************************************************************
// Static functions
//**************************************************************************************************

bool transferGridCellData( RigMainGrid*         mainGrid,
                           RigActiveCellInfo*   activeCellInfo,
                           RigActiveCellInfo*   fractureActiveCellInfo,
                           RigGridBase*         localGrid,
                           const ecl_grid_type* localEclGrid,
                           size_t               matrixActiveStartIndex,
                           size_t               fractureActiveStartIndex )
{
    CVF_ASSERT( activeCellInfo && fractureActiveCellInfo );

    int    cellCount      = ecl_grid_get_global_size( localEclGrid );
    size_t cellStartIndex = mainGrid->globalCellArray().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid( localGrid );
    mainGrid->globalCellArray().resize( cellStartIndex + cellCount, defaultCell );

    mainGrid->nodes().resize( nodeStartIndex + cellCount * 8, cvf::Vec3d( 0, 0, 0 ) );

    // Loop over cells and fill them with data

#pragma omp parallel for
    for ( int gridLocalCellIndex = 0; gridLocalCellIndex < cellCount; ++gridLocalCellIndex )
    {
        RigCell& cell = mainGrid->globalCellArray()[cellStartIndex + gridLocalCellIndex];

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
        cell.setInvalid( cell.isLongPyramidCell() );
    }

    return true;
}

//==================================================================================================
//
// Class RigReaderInterfaceECL
//
//==================================================================================================

//--------------------------------------------------------------------------------------------------
/// Constructor
//--------------------------------------------------------------------------------------------------
RifReaderEclipseOutput::RifReaderEclipseOutput()
{
    m_fileName.clear();
    m_filesWithSameBaseName.clear();

    m_eclipseCase = nullptr;

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
/// Read geometry from file given by name into given reservoir object
//--------------------------------------------------------------------------------------------------
bool RifReaderEclipseOutput::transferGeometry( const ecl_grid_type* mainEclGrid, RigEclipseCaseData* eclipseCase )
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
    {
        cvf::Vec3st gridPointDim( 0, 0, 0 );
        gridPointDim.x() = ecl_grid_get_nx( mainEclGrid ) + 1;
        gridPointDim.y() = ecl_grid_get_ny( mainEclGrid ) + 1;
        gridPointDim.z() = ecl_grid_get_nz( mainEclGrid ) + 1;
        mainGrid->setGridPointDimensions( gridPointDim );
    }

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

        cvf::Vec3st gridPointDim( 0, 0, 0 );
        gridPointDim.x() = ecl_grid_get_nx( localEclGrid ) + 1;
        gridPointDim.y() = ecl_grid_get_ny( localEclGrid ) + 1;
        gridPointDim.z() = ecl_grid_get_nz( localEclGrid ) + 1;

        RigLocalGrid* localGrid = new RigLocalGrid( mainGrid );
        localGrid->setGridId( lgrId );
        mainGrid->addLocalGrid( localGrid );

        localGrid->setIndexToStartOfCells( totalCellCount );
        localGrid->setGridName( lgrName );
        localGrid->setGridPointDimensions( gridPointDim );

        totalCellCount += ecl_grid_get_global_size( localEclGrid );
    }

    activeCellInfo->setReservoirCellCount( totalCellCount );
    fractureActiveCellInfo->setReservoirCellCount( totalCellCount );

    // Reserve room for the cells and nodes and fill them with data

    mainGrid->globalCellArray().reserve( totalCellCount );
    mainGrid->nodes().reserve( 8 * totalCellCount );

    caf::ProgressInfo progInfo( 3 + numLGRs, "" );

    {
        auto task = progInfo.task( "Loading Main Grid Data", 3 );
        transferGridCellData( mainGrid, activeCellInfo, fractureActiveCellInfo, mainGrid, mainEclGrid, 0, 0 );
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

        transferGridCellData( mainGrid, activeCellInfo, fractureActiveCellInfo, localGrid, localEclGrid, globalMatrixActiveSize, globalFractureActiveSize );

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
bool RifReaderEclipseOutput::open( const QString& fileName, RigEclipseCaseData* eclipseCase )
{
    CVF_ASSERT( eclipseCase );
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
        if ( !transferGeometry( mainEclGrid, eclipseCase ) ) return false;

        RifOpmGridTools::importCoordinatesForRadialGrid( fileName.toStdString(), eclipseCase->mainGrid() );
    }

    {
        auto task = progress.task( "Reading faults", 10 );

        if ( isFaultImportEnabled() )
        {
            cvf::Collection<RigFault> faults;

            importFaults( fileSet, &faults );

            RigMainGrid* mainGrid = eclipseCase->mainGrid();
            mainGrid->setFaults( faults );
        }
    }

    m_eclipseCase = eclipseCase;

    {
        auto task = progress.task( "Reading Results Meta data", 25 );
        buildMetaData( mainEclGrid );
    }

    {
        auto task = progress.task( "Handling NCC data", 20 );
        if ( isNNCsEnabled() )
        {
            caf::ProgressInfo nncProgress( 10, "" );
            RigMainGrid*      mainGrid = eclipseCase->mainGrid();

            {
                auto subNncTask = nncProgress.task( "Reading static NNC data" );
                transferStaticNNCData( mainEclGrid, m_ecl_init_file, mainGrid );
            }

            // This test should probably be improved to test more directly for presence of NNC data
            if ( m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasFlowDiagUsableFluxes() )
            {
                auto subNncTask = nncProgress.task( "Reading dynamic NNC data" );
                transferDynamicNNCData( mainEclGrid, mainGrid );
            }

            RigActiveCellInfo* activeCellInfo = m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

            bool includeInactiveCells = includeInactiveCellsInFaultGeometry();
            mainGrid->nncData()->setSourceDataForProcessing( mainGrid, activeCellInfo, includeInactiveCells );
        }
    }

    {
        auto task = progress.task( "Handling well information", 10 );
        if ( !isSkipWellData() )
        {
            readWellCells( mainEclGrid, isImportOfCompleteMswDataEnabled() );
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
    CVF_ASSERT( m_eclipseCase );

    RigCaseCellResultsData* matrixModelResults = m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
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
    m_eclipseCase           = eclipseCase;
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
    CVF_ASSERT( m_eclipseCase );
    CVF_ASSERT( m_eclipseCase->mainGrid() );

    std::vector<std::vector<int>> actnumValuesPerGrid;

    {
        // If INIT file is present and PORV is found, use PORV as basis for active cells
        QString initFileName = RifEclipseOutputFileTools::firstFileNameOfType( m_filesWithSameBaseName, ECL_INIT_FILE );
        if ( initFileName.size() > 0 )
        {
            ecl_file_type* ecl_file = ecl_file_open( RiaStringEncodingTools::toNativeEncoded( initFileName ).data(), ECL_FILE_CLOSE_STREAM );
            if ( ecl_file )
            {
                bool isDualPorosity    = m_eclipseCase->mainGrid()->isDualPorosity();
                int  cellCountMainGrid = static_cast<int>( m_eclipseCase->mainGrid()->cellCount() );
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

    return RifEclipseOutputFileTools::assignActiveCellData( actnumValuesPerGrid, m_eclipseCase );
}

//--------------------------------------------------------------------------------------------------
/// Build meta data - get states and results info
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::buildMetaData( ecl_grid_type* grid )
{
    CVF_ASSERT( m_eclipseCase );
    CVF_ASSERT( !m_filesWithSameBaseName.empty() );

    caf::ProgressInfo progInfo( m_filesWithSameBaseName.size() + 3, "" );

    progInfo.setNextProgressIncrement( m_filesWithSameBaseName.size() );

    RigCaseCellResultsData* matrixModelResults   = m_eclipseCase->results( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigCaseCellResultsData* fractureModelResults = m_eclipseCase->results( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    std::vector<RigEclipseTimeStepInfo> timeStepInfos;

    // Create access object for dynamic results
    ensureDynamicResultAccessIsPresent();
    if ( m_dynamicResultsAccess.notNull() )
    {
        m_dynamicResultsAccess->open();

        progInfo.incrementProgress();

        timeStepInfos = createFilteredTimeStepInfos();

        auto keywordValueCounts = m_dynamicResultsAccess->keywordValueCounts();

        {
            auto validKeywords = validKeywordsForPorosityModel( keywordValueCounts,
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ),
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL ),
                                                                RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                m_dynamicResultsAccess->timeStepCount() );

            for ( const auto& keywordData : validKeywords )
            {
                RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                 RifKeywordValueCount::mapType( keywordData.dataType() ),
                                                 QString::fromStdString( keywordData.keyword() ) );
                matrixModelResults->createResultEntry( resAddr, false );
                matrixModelResults->setTimeStepInfos( resAddr, timeStepInfos );
            }
        }

        {
            auto validKeywords = validKeywordsForPorosityModel( keywordValueCounts,
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ),
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL ),
                                                                RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                m_dynamicResultsAccess->timeStepCount() );

            for ( const auto& keywordData : validKeywords )
            {
                RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                 RifKeywordValueCount::mapType( keywordData.dataType() ),
                                                 QString::fromStdString( keywordData.keyword() ) );
                fractureModelResults->createResultEntry( resAddr, false );
                fractureModelResults->setTimeStepInfos( resAddr, timeStepInfos );
            }
        }
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
        m_eclipseCase->setUnitsType( unitsType );
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

        {
            auto validKeywords = validKeywordsForPorosityModel( keywordInfo,
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ),
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL ),
                                                                RiaDefines::PorosityModelType::MATRIX_MODEL,
                                                                1 );

            validKeywords.push_back( RifKeywordValueCount( "ACTNUM", 0, RifKeywordValueCount::KeywordDataType::INTEGER ) );

            for ( const auto& keywordData : validKeywords )
            {
                RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                 RifKeywordValueCount::mapType( keywordData.dataType() ),
                                                 QString::fromStdString( keywordData.keyword() ) );
                matrixModelResults->createResultEntry( resAddr, false );
                matrixModelResults->setTimeStepInfos( resAddr, staticTimeStepInfo );
            }
        }

        {
            auto validKeywords = validKeywordsForPorosityModel( keywordInfo,
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL ),
                                                                m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL ),
                                                                RiaDefines::PorosityModelType::FRACTURE_MODEL,
                                                                1 );
            validKeywords.push_back( RifKeywordValueCount( "ACTNUM", 0, RifKeywordValueCount::KeywordDataType::INTEGER ) );

            for ( const auto& keywordData : validKeywords )
            {
                RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::STATIC_NATIVE,
                                                 RifKeywordValueCount::mapType( keywordData.dataType() ),
                                                 QString::fromStdString( keywordData.keyword() ) );
                fractureModelResults->createResultEntry( resAddr, false );
                fractureModelResults->setTimeStepInfos( resAddr, staticTimeStepInfo );
            }
        }
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
        RigActiveCellInfo* activeCellInfo = m_eclipseCase->activeCellInfo( matrixOrFracture );
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

        extractResultValuesBasedOnPorosityModel( matrixOrFracture, values, fileValues );
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

    if ( m_eclipseCase->mainGrid()->gridCount() == 0 )
    {
        RiaLogging::error( "No grids available" );

        return;
    }

    RigActiveCellInfo* fracActCellInfo = m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
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
        if ( !m_dynamicResultsAccess->results( result, indexOnFile, m_eclipseCase->mainGrid()->gridCountOnFile(), &fileValues ) )
        {
            return false;
        }

        extractResultValuesBasedOnPorosityModel( matrixOrFracture, values, fileValues );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// Helper struct to store info on how a well-to-grid connection contributes to the position of
/// well segments without any connections.
//--------------------------------------------------------------------------------------------------
struct SegmentPositionContribution
{
    SegmentPositionContribution( int        connectionSegmentId,
                                 cvf::Vec3d connectionPosition,
                                 double     lengthFromConnection,
                                 bool       isInsolating,
                                 int        segmentIdUnder,
                                 int        segmentIdAbove,
                                 bool       isFromAbove )
        : m_connectionSegmentId( connectionSegmentId )
        , m_lengthFromConnection( lengthFromConnection )
        , m_isInsolating( isInsolating )
        , m_connectionPosition( connectionPosition )
        , m_segmentIdUnder( segmentIdUnder )
        , m_segmentIdAbove( segmentIdAbove )
        , m_isFromAbove( isFromAbove )
    {
    }

    int        m_connectionSegmentId;
    double     m_lengthFromConnection;
    bool       m_isInsolating;
    cvf::Vec3d m_connectionPosition;
    int        m_segmentIdUnder;
    int        m_segmentIdAbove;
    bool       m_isFromAbove;
};

size_t localGridCellIndexFromErtConnection( const RigGridBase* grid, const well_conn_type* ert_connection, const char* wellNameForErrorMsgs )
{
    CVF_ASSERT( ert_connection );
    CVF_ASSERT( grid );

    int cellI = well_conn_get_i( ert_connection );
    int cellJ = well_conn_get_j( ert_connection );
    int cellK = well_conn_get_k( ert_connection );

    // If a well is defined in fracture region, the K-value is from (cellCountK - 1) -> cellCountK*2 - 1
    // Adjust K so index is always in valid grid region
    if ( cellK >= static_cast<int>( grid->cellCountK() ) )
    {
        cellK -= static_cast<int>( grid->cellCountK() );
    }

    // See description for keyword ICON at page 54/55 of Rile Formats Reference Manual 2010.2
    /*
    Integer completion data array
    ICON(NICONZ,NCWMAX,NWELLS) with dimensions
    defined by INTEHEAD. The following items are required for each completion in each well:
    Item 1 - Well connection index ICON(1,IC,IWELL) = IC (set to -IC if connection is not in current LGR)
    Item 2 - I-coordinate (<= 0 if not in this LGR)
    Item 3 - J-coordinate (<= 0 if not in this LGR)
    Item 4 - K-coordinate (<= 0 if not in this LGR)
    Item 6 - Connection status > 0 open, <= 0 shut
    Item 14 - Penetration direction (1=x, 2=y, 3=z, 4=fractured in x-direction, 5=fractured in y-direction)
    If undefined or zero, assume Z
    Item 15 - Segment number containing connection (for multi-segment wells, =0 for ordinary wells)
    Undefined items in this array may be set to zero.
    */

    // The K value might also be -1. It is not yet known why, or what it is supposed to mean,
    // but for now we will interpret as 0.
    // TODO: Ask Joakim Haave regarding this.
    if ( cellK < 0 )
    {
        // cvf::Trace::show("Well Connection for grid " + cvf::String(grid->gridName()) + "\n - Detected negative K
        // value (K=" + cvf::String(cellK) + ") for well : " + cvf::String(wellName) + " K clamped to 0");

        cellK = 0;
    }

    // Introduced based on discussion with H�kon H�gst�l 08.09.2016
    if ( cellK >= static_cast<int>( grid->cellCountK() ) )
    {
        int maxCellK = static_cast<int>( grid->cellCountK() );
        if ( wellNameForErrorMsgs )
        {
            cvf::Trace::show( "Well Connection for grid " + cvf::String( grid->gridName() ) +
                              "\n - Ignored connection with invalid K value (K=" + cvf::String( cellK ) +
                              ", max K = " + cvf::String( maxCellK ) + ") for well : " + cvf::String( wellNameForErrorMsgs ) );
        }
        return cvf::UNDEFINED_SIZE_T;
    }

    return grid->cellIndexFromIJK( cellI, cellJ, cellK );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint RifReaderEclipseOutput::createWellResultPoint( const RigGridBase*       grid,
                                                                  const well_conn_type*    ert_connection,
                                                                  const well_segment_type* segment,
                                                                  const char*              wellName )
{
    CVF_ASSERT( ert_connection );
    CVF_ASSERT( grid );

    size_t gridCellIndex = localGridCellIndexFromErtConnection( grid, ert_connection, wellName );

    bool   isCellOpen       = well_conn_open( ert_connection );
    double volumeRate       = well_conn_get_volume_rate( ert_connection );
    double oilRate          = well_conn_get_oil_rate( ert_connection );
    double gasRate          = well_conn_get_gas_rate( ert_connection );
    double waterRate        = well_conn_get_water_rate( ert_connection );
    double connectionFactor = well_conn_get_connection_factor( ert_connection );

    RigWellResultPoint resultPoint;

    if ( gridCellIndex != cvf::UNDEFINED_SIZE_T )
    {
        int branchId = -1, segmentId = -1, outletBranchId = -1, outletSegmentId = -1;

        if ( segment )
        {
            branchId  = well_segment_get_branch_id( segment );
            segmentId = well_segment_get_id( segment );

            auto outletSegment = well_segment_get_outlet( segment );
            if ( outletSegment )
            {
                outletBranchId  = well_segment_get_branch_id( outletSegment );
                outletSegmentId = well_segment_get_id( outletSegment );
            }
        }

        resultPoint.setGridIndex( grid->gridIndex() );
        resultPoint.setGridCellIndex( gridCellIndex );

        resultPoint.setIsOpen( isCellOpen );

        resultPoint.setSegmentData( branchId, segmentId );
        resultPoint.setOutletSegmentData( outletBranchId, outletSegmentId );

        const double adjustedGasRate = RiaEclipseUnitTools::convertSurfaceGasFlowRateToOilEquivalents( m_eclipseCase->unitsType(), gasRate );
        resultPoint.setFlowData( volumeRate, oilRate, adjustedGasRate, waterRate );

        resultPoint.setConnectionFactor( connectionFactor );

        auto ijkOneBased = grid->ijkFromCellIndexOneBased( gridCellIndex );
        if ( ijkOneBased )
        {
            resultPoint.setIjk( *ijkOneBased );
        }
    }

    return resultPoint;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigWellResultPoint
    RifReaderEclipseOutput::createWellResultPoint( const RigGridBase* grid, const well_conn_type* ert_connection, const char* wellName )
{
    return createWellResultPoint( grid, ert_connection, nullptr, wellName );
}

//--------------------------------------------------------------------------------------------------
/// Inverse distance interpolation of the supplied points and distance weights for
/// the contributing points which are closest above, and closest below
//--------------------------------------------------------------------------------------------------
cvf::Vec3d interpolate3DPosition( const std::vector<SegmentPositionContribution>& positions )
{
    std::vector<SegmentPositionContribution> filteredPositions;
    filteredPositions.reserve( positions.size() );

    double                                   minDistFromContribAbove = HUGE_VAL;
    double                                   minDistFromContribBelow = HUGE_VAL;
    std::vector<SegmentPositionContribution> contrFromAbove;
    std::vector<SegmentPositionContribution> contrFromBelow;

    for ( size_t i = 0; i < positions.size(); i++ )
    {
        if ( positions[i].m_connectionPosition != cvf::Vec3d::UNDEFINED )
        {
            if ( positions[i].m_isFromAbove && positions[i].m_lengthFromConnection < minDistFromContribAbove )
            {
                if ( !contrFromAbove.empty() )
                    contrFromAbove[0] = positions[i];
                else
                    contrFromAbove.push_back( positions[i] );

                minDistFromContribAbove = positions[i].m_lengthFromConnection;
            }

            if ( !positions[i].m_isFromAbove && positions[i].m_lengthFromConnection < minDistFromContribBelow )
            {
                if ( !contrFromBelow.empty() )
                    contrFromBelow[0] = positions[i];
                else
                    contrFromBelow.push_back( positions[i] );

                minDistFromContribBelow = positions[i].m_lengthFromConnection;
            }
        }
    }

    filteredPositions = contrFromAbove;
    filteredPositions.insert( filteredPositions.end(), contrFromBelow.begin(), contrFromBelow.end() );

    std::vector<double> nominators( filteredPositions.size(), 0.0 );

    double     denominator       = 0.0;
    cvf::Vec3d interpolatedValue = cvf::Vec3d::ZERO;

    for ( size_t i = 0; i < filteredPositions.size(); i++ )
    {
#if 0 // Pure average test
        nominators[i] = 1.0;
#else
        double distance = filteredPositions[i].m_lengthFromConnection;

        if ( distance < 1e-6 )
        {
            return filteredPositions[i].m_connectionPosition;
        }
        else if ( distance < 1.0 )
        {
            // distance = 1.0;
        }

        distance      = 1.0 / distance;
        nominators[i] = distance;
        denominator += distance;

#endif
    }
#if 0 // Pure average test
    denominator = positions.size(); // Pure average test
#endif
    for ( size_t i = 0; i < filteredPositions.size(); i++ )
    {
        interpolatedValue += ( nominators[i] / denominator ) * filteredPositions[i].m_connectionPosition;
    }

    return interpolatedValue;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void propagatePosContribDownwards( std::map<int, std::vector<SegmentPositionContribution>>& segmentIdToPositionContrib,
                                   const well_segment_collection_type*                      allErtSegments,
                                   int                                                      ertSegmentId,
                                   std::vector<SegmentPositionContribution>                 posContrib )
{
    std::map<int, std::vector<SegmentPositionContribution>>::iterator posContribIt;
    posContribIt = segmentIdToPositionContrib.find( ertSegmentId );

    if ( posContribIt != segmentIdToPositionContrib.end() )
    {
        // Create a set of the segments below this, that has to be followed.

        std::set<int> segmentIdsBelow;
        for ( size_t i = 0; i < posContribIt->second.size(); ++i )
        {
            segmentIdsBelow.insert( posContribIt->second[i].m_segmentIdUnder );
        }

        // Get the segment length to add to the contributions

        well_segment_type* segment      = well_segment_collection_get( allErtSegments, posContribIt->first );
        double             sementLength = well_segment_get_length( segment );

        // If we do not have the contribution represented, add it, and accumulate the length
        // If it is already present, do not touch
        for ( size_t i = 0; i < posContrib.size(); ++i )
        {
            bool foundContribution = false;
            for ( size_t j = 0; j < posContribIt->second.size(); ++j )
            {
                if ( posContribIt->second[j].m_connectionSegmentId == posContrib[i].m_connectionSegmentId )
                {
                    foundContribution = true;
                    break;
                }
            }

            if ( !foundContribution )
            {
                posContrib[i].m_lengthFromConnection += sementLength;
                posContrib[i].m_isFromAbove = true;
                posContribIt->second.push_back( posContrib[i] );
            }
            posContrib[i].m_segmentIdAbove = ertSegmentId;
        }

        for ( std::set<int>::iterator it = segmentIdsBelow.begin(); it != segmentIdsBelow.end(); ++it )
        {
            propagatePosContribDownwards( segmentIdToPositionContrib, allErtSegments, ( *it ), posContrib );
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// Helper class to determine whether a well connection is present in a sub cell
//  for a specific well. Connections must be tested from innermost lgr to outermost since
//  it accumulates the outer cells having subcell connections as it goes.
//--------------------------------------------------------------------------------------------------
class WellResultPointHasSubCellConnectionCalculator
{
public:
    explicit WellResultPointHasSubCellConnectionCalculator( const RigMainGrid* mainGrid, well_state_type* ert_well_state )
        : m_mainGrid( mainGrid )
    {
        int lastGridNr = static_cast<int>( m_mainGrid->gridCountOnFile() ) - 1;

        for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
        {
            const well_conn_type* ert_wellhead = well_state_iget_wellhead( ert_well_state, static_cast<int>( gridNr ) );
            if ( ert_wellhead )
            {
                size_t localGridCellidx = localGridCellIndexFromErtConnection( m_mainGrid->gridByIndex( gridNr ), ert_wellhead, nullptr );
                insertTheParentCells( gridNr, localGridCellidx );
            }

            std::string                      gridname = gridNr == 0 ? ECL_GRID_GLOBAL_GRID : m_mainGrid->gridByIndex( gridNr )->gridName();
            const well_conn_collection_type* connections = well_state_get_grid_connections( ert_well_state, gridname.data() );

            if ( connections )
            {
                int connectionCount = well_conn_collection_get_size( connections );
                if ( connectionCount )
                {
                    for ( int connIdx = 0; connIdx < connectionCount; connIdx++ )
                    {
                        well_conn_type* ert_connection = well_conn_collection_iget( connections, connIdx );

                        size_t localGridCellidx =
                            localGridCellIndexFromErtConnection( m_mainGrid->gridByIndex( gridNr ), ert_connection, nullptr );
                        insertTheParentCells( gridNr, localGridCellidx );
                    }
                }
            }
        }
    }

    bool hasSubCellConnection( const RigWellResultPoint& wellResultPoint )
    {
        if ( !wellResultPoint.isCell() ) return false;

        size_t gridIndex     = wellResultPoint.gridIndex();
        size_t gridCellIndex = wellResultPoint.cellIndex();

        size_t reservoirCellIdx = m_mainGrid->reservoirCellIndexByGridAndGridLocalCellIndex( gridIndex, gridCellIndex );

        return m_gridCellsWithSubCellWellConnections.count( reservoirCellIdx ) != 0;
    }

private:
    void insertTheParentCells( size_t gridIndex, size_t gridCellIndex )
    {
        if ( gridCellIndex == cvf::UNDEFINED_SIZE_T ) return;

        // Traverse parent gridcells, and add them to the map

        while ( gridIndex > 0 ) // is lgr
        {
            const RigCell& connectionCell = m_mainGrid->cellByGridAndGridLocalCellIdx( gridIndex, gridCellIndex );
            RigGridBase*   hostGrid       = connectionCell.hostGrid();

            RigLocalGrid* lgrHost = static_cast<RigLocalGrid*>( hostGrid );
            gridIndex             = lgrHost->parentGrid()->gridIndex();
            gridCellIndex         = connectionCell.parentCellIndex();

            size_t parentReservoirCellIdx = m_mainGrid->reservoirCellIndexByGridAndGridLocalCellIndex( gridIndex, gridCellIndex );
            m_gridCellsWithSubCellWellConnections.insert( parentReservoirCellIdx );
        }
    }

    std::set<size_t>   m_gridCellsWithSubCellWellConnections;
    const RigMainGrid* m_mainGrid;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderEclipseOutput::readWellCells( const ecl_grid_type* mainEclGrid, bool importCompleteMswData )
{
    CVF_ASSERT( m_eclipseCase );

    if ( m_dynamicResultsAccess.isNull() ) return;

    well_info_type* ert_well_info = well_info_alloc( mainEclGrid );
    if ( !ert_well_info ) return;

    m_dynamicResultsAccess->readWellData( ert_well_info, importCompleteMswData );

    std::vector<double>    daysSinceSimulationStart;
    std::vector<QDateTime> timeSteps;
    m_dynamicResultsAccess->timeSteps( &timeSteps, &daysSinceSimulationStart );
    std::vector<int> reportNumbers = m_dynamicResultsAccess->reportNumbers();

    bool sameCount = false;
    if ( timeSteps.size() == reportNumbers.size() )
    {
        sameCount = true;
    }

    std::vector<RigGridBase*> grids;
    m_eclipseCase->allGrids( &grids );

    cvf::Collection<RigSimWellData> wells;
    caf::ProgressInfo               progress( well_info_get_num_wells( ert_well_info ), "" );

    int wellIdx;
    for ( wellIdx = 0; wellIdx < well_info_get_num_wells( ert_well_info ); wellIdx++ )
    {
        const char* wellName = well_info_iget_well_name( ert_well_info, wellIdx );
        CVF_ASSERT( wellName );

        cvf::ref<RigSimWellData> simWellData = new RigSimWellData;
        simWellData->m_wellName              = wellName;

        well_ts_type* ert_well_time_series = well_info_get_ts( ert_well_info, wellName );
        int           timeStepCount        = well_ts_get_size( ert_well_time_series );

        simWellData->m_wellCellsTimeSteps.resize( timeStepCount );

        int timeIdx;
        for ( timeIdx = 0; timeIdx < timeStepCount; timeIdx++ )
        {
            well_state_type* ert_well_state = well_ts_iget_state( ert_well_time_series, timeIdx );

            RigWellResultFrame& wellResFrame = simWellData->m_wellCellsTimeSteps[timeIdx];

            // Build timestamp for well
            bool haveFoundTimeStamp = false;

            if ( sameCount )
            {
                int reportNr = well_state_get_report_nr( ert_well_state );

                for ( size_t i = 0; i < reportNumbers.size(); i++ )
                {
                    if ( reportNumbers[i] == reportNr )
                    {
                        wellResFrame.setTimestamp( timeSteps[i] );
                        haveFoundTimeStamp = true;
                    }
                }
            }

            if ( !haveFoundTimeStamp )
            {
                // This fallback will not work for timesteps before 1970.

                // Also see RifEclipseOutputFileAccess::timeStepsText for accessing time_t structures
                time_t stepTime = well_state_get_sim_time( ert_well_state );
                wellResFrame.setTimestamp( QDateTime::fromSecsSinceEpoch( stepTime ) );
            }

            // Production type
            well_type_enum ert_well_type = well_state_get_type( ert_well_state );
            if ( ert_well_type == ECL_WELL_PRODUCER )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::PRODUCER );
            }
            else if ( ert_well_type == ECL_WELL_WATER_INJECTOR )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::WATER_INJECTOR );
            }
            else if ( ert_well_type == ECL_WELL_GAS_INJECTOR )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::GAS_INJECTOR );
            }
            else if ( ert_well_type == ECL_WELL_OIL_INJECTOR )
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::OIL_INJECTOR );
            }
            else
            {
                wellResFrame.setProductionType( RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE );
            }

            wellResFrame.setIsOpen( well_state_is_open( ert_well_state ) );

            if ( importCompleteMswData && well_state_is_MSW( ert_well_state ) )
            {
                simWellData->setMultiSegmentWell( true );

                // how do we handle LGR-s ?
                // 1. Create separate visual branches for each Grid, with its own wellhead
                // 2. Always use the connections to the grid with the highest number (innermost LGR).
                // 3. Handle both and switch between them according to visual settings of grid visualization
                // Will there ever exist connections to different grids for the same segment ?
                // We have currently selected 2.

                // Set the wellhead

                int lastGridNr = static_cast<int>( grids.size() ) - 1;
                for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                {
                    //  If several grids have a wellhead definition for this well, we use the last one.
                    // (Possibly the innermost LGR)

                    const well_conn_type* ert_wellhead = well_state_iget_wellhead( ert_well_state, static_cast<int>( gridNr ) );
                    if ( ert_wellhead )
                    {
                        auto wellHead = createWellResultPoint( grids[gridNr], ert_wellhead, wellName );

                        // HACK: Ert returns open as "this is equally wrong as closed for well heads".
                        // Well heads are not open jfr mail communication with HHGS and JH Statoil 07.01.2016
                        wellHead.setIsOpen( false );
                        wellResFrame.setWellHead( wellHead );
                        break;
                    }
                }

                well_branch_collection_type*                            branches    = well_state_get_branches( ert_well_state );
                int                                                     branchCount = well_branch_collection_get_size( branches );
                std::map<int, std::vector<SegmentPositionContribution>> segmentIdToPositionContrib;
                std::vector<int>                                        upperSegmentIdsOfUnpositionedSegementGroup;

                // Create copy of well result branches for modification
                std::vector<RigWellResultBranch> wellResultBranches = wellResFrame.wellResultBranches();
                wellResultBranches.resize( branchCount );

                // For each branch, go from bottom segment upwards and transfer their connections to WellResultpoints.
                // If they have no connections, create a resultpoint representing their bottom position, which will
                // receive an actual position at a later stage.
                // I addition, distribute contributions for calculating segment bottom positions from bottom and up.

                for ( int bIdx = 0; bIdx < well_branch_collection_get_size( branches ); bIdx++ )
                {
                    RigWellResultBranch& wellResultBranch = wellResultBranches[bIdx];

                    const well_segment_type* segment = well_branch_collection_iget_start_segment( branches, bIdx );

                    int branchId = well_segment_get_branch_id( segment );
                    wellResultBranch.setErtBranchId( branchId );

                    // Data for segment position calculation
                    int        lastConnectionSegmentId     = -1;
                    cvf::Vec3d lastConnectionPos           = cvf::Vec3d::UNDEFINED;
                    cvf::Vec3d lastConnectionCellCorner    = cvf::Vec3d::UNDEFINED;
                    double     lastConnectionCellSize      = 0;
                    double     accLengthFromLastConnection = 0;
                    int        segmentIdBelow              = -1;
                    bool       segmentBelowHasConnections  = false;

                    while ( segment && branchId == well_segment_get_branch_id( segment ) )
                    {
                        // Loop backwards, making us select the connection in the innermost lgr as the truth
                        bool segmentHasConnections = false;

                        for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                        {
                            std::string gridName = ertGridName( gridNr );

                            // If this segment has connections in any grid, transfer the innermost ones

                            if ( well_segment_has_grid_connections( segment, gridName.data() ) )
                            {
                                const well_conn_collection_type* connections     = well_segment_get_connections( segment, gridName.data() );
                                int                              connectionCount = well_conn_collection_get_size( connections );

                                // Loop backwards to put the deepest connections first in the array. (The segments are
                                // also traversed deep to shallow)
                                for ( int connIdx = connectionCount - 1; connIdx >= 0; connIdx-- )
                                {
                                    well_conn_type* ert_connection = well_conn_collection_iget( connections, connIdx );
                                    wellResultBranch.addBranchResultPoint(
                                        createWellResultPoint( grids[gridNr], ert_connection, segment, wellName ) );
                                }

                                segmentHasConnections = true;

                                // Prepare data for segment position calculation

                                well_conn_type*    ert_connection = well_conn_collection_iget( connections, 0 );
                                RigWellResultPoint point = createWellResultPoint( grids[gridNr], ert_connection, segment, wellName );
                                lastConnectionPos        = grids[gridNr]->cell( point.cellIndex() ).center();
                                cvf::Vec3d cellVxes[8];
                                grids[gridNr]->cellCornerVertices( point.cellIndex(), cellVxes );
                                lastConnectionCellCorner = cellVxes[0];
                                lastConnectionCellSize   = ( lastConnectionPos - cellVxes[0] ).length();

                                lastConnectionSegmentId     = well_segment_get_id( segment );
                                accLengthFromLastConnection = well_segment_get_length( segment ) / ( connectionCount + 1 );
                                if ( !segmentBelowHasConnections ) upperSegmentIdsOfUnpositionedSegementGroup.push_back( segmentIdBelow );

                                break; // Stop looping over grids
                            }
                        }

                        // If the segment did not have connections at all, we need to create a resultpoint representing
                        // the bottom of the segment and store it as an unpositioned segment

                        if ( !segmentHasConnections )
                        {
                            RigWellResultPoint data;
                            data.setSegmentData( branchId, well_segment_get_id( segment ) );

                            wellResultBranch.addBranchResultPoint( data );

                            // Store data for segment position calculation
                            bool isAnInsolationContribution = accLengthFromLastConnection < lastConnectionCellSize;

                            segmentIdToPositionContrib[well_segment_get_id( segment )].push_back(
                                SegmentPositionContribution( lastConnectionSegmentId,
                                                             lastConnectionPos,
                                                             accLengthFromLastConnection,
                                                             isAnInsolationContribution,
                                                             segmentIdBelow,
                                                             -1,
                                                             false ) );
                            accLengthFromLastConnection += well_segment_get_length( segment );
                        }

                        segmentIdBelow             = well_segment_get_id( segment );
                        segmentBelowHasConnections = segmentHasConnections;

                        if ( well_segment_get_outlet_id( segment ) == -1 )
                        {
                            segment = nullptr;
                        }
                        else
                        {
                            segment = well_segment_get_outlet( segment );
                        }
                    }

                    // Add resultpoint representing the outlet segment (bottom), if not the branch ends at the wellhead.

                    const well_segment_type* outletSegment = segment;

                    if ( outletSegment )
                    {
                        bool outletSegmentHasConnections = false;

                        for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                        {
                            std::string gridName = ertGridName( gridNr );

                            // If this segment has connections in any grid, use the deepest innermost one

                            if ( well_segment_has_grid_connections( outletSegment, gridName.data() ) )
                            {
                                const well_conn_collection_type* connections = well_segment_get_connections( outletSegment, gridName.data() );
                                int connectionCount = well_conn_collection_get_size( connections );

                                // Select the deepest connection
                                well_conn_type* ert_connection = well_conn_collection_iget( connections, connectionCount - 1 );

                                auto resultPoint = createWellResultPoint( grids[gridNr], ert_connection, outletSegment, wellName );
                                // This result point is only supposed to be used to indicate connection to a parent well
                                // Clear all flow in this result point
                                resultPoint.clearAllFlow();

                                wellResultBranch.addBranchResultPoint( resultPoint );

                                outletSegmentHasConnections = true;
                                break; // Stop looping over grids
                            }
                        }

                        if ( !outletSegmentHasConnections )
                        {
                            // Store the result point

                            RigWellResultPoint data;
                            data.setSegmentData( well_segment_get_branch_id( outletSegment ), well_segment_get_id( outletSegment ) );
                            wellResultBranch.addBranchResultPoint( data );

                            // Store data for segment position calculation,
                            // and propagate it upwards until we meet a segment with connections

                            bool isAnInsolationContribution = accLengthFromLastConnection < lastConnectionCellSize;

                            cvf::Vec3d lastConnectionPosWOffset = lastConnectionPos;
                            if ( isAnInsolationContribution )
                                lastConnectionPosWOffset += 0.4 * ( lastConnectionCellCorner - lastConnectionPos );

                            segmentIdToPositionContrib[well_segment_get_id( outletSegment )].push_back(
                                SegmentPositionContribution( lastConnectionSegmentId,
                                                             lastConnectionPosWOffset,
                                                             accLengthFromLastConnection,
                                                             isAnInsolationContribution,
                                                             segmentIdBelow,
                                                             -1,
                                                             false ) );

                            /// Loop further to add this position contribution until a segment with connections is found

                            accLengthFromLastConnection += well_segment_get_length( outletSegment );
                            segmentIdBelow = well_segment_get_id( outletSegment );

                            const well_segment_type* aboveOutletSegment = nullptr;

                            if ( well_segment_get_outlet_id( outletSegment ) == -1 )
                            {
                                aboveOutletSegment = nullptr;
                            }
                            else
                            {
                                aboveOutletSegment = well_segment_get_outlet( outletSegment );
                            }

                            while ( aboveOutletSegment )
                            {
                                // Loop backwards, just because we do that the other places
                                bool segmentHasConnections = false;

                                for ( int gridNr = lastGridNr; gridNr >= 0; --gridNr )
                                {
                                    std::string gridName = ertGridName( gridNr );

                                    // If this segment has connections in any grid, stop traversal

                                    if ( well_segment_has_grid_connections( aboveOutletSegment, gridName.data() ) )
                                    {
                                        segmentHasConnections = true;
                                        break;
                                    }
                                }

                                if ( !segmentHasConnections )
                                {
                                    segmentIdToPositionContrib[well_segment_get_id( aboveOutletSegment )].push_back(
                                        SegmentPositionContribution( lastConnectionSegmentId,
                                                                     lastConnectionPos,
                                                                     accLengthFromLastConnection,
                                                                     isAnInsolationContribution,
                                                                     segmentIdBelow,
                                                                     -1,
                                                                     false ) );
                                    accLengthFromLastConnection += well_segment_get_length( aboveOutletSegment );
                                }
                                else
                                {
                                    break; // We have found a segment with connections. We do not need to propagate
                                           // position contributions further
                                }

                                segmentIdBelow = well_segment_get_id( aboveOutletSegment );

                                if ( well_segment_get_outlet_id( aboveOutletSegment ) == -1 )
                                {
                                    aboveOutletSegment = nullptr;
                                }
                                else
                                {
                                    aboveOutletSegment = well_segment_get_outlet( aboveOutletSegment );
                                }
                            }
                        }
                    }
                    else
                    {
                        // Add wellhead as result point Nope. Not Yet, but it is a good idea.
                        // The centerline calculations would be a bit simpler, I think.
                    }

                    // Reverse the order of the result points in this branch, making the deepest come last
                    auto branchResultPoints = wellResultBranch.branchResultPoints();
                    std::reverse( branchResultPoints.begin(), branchResultPoints.end() );
                    wellResultBranch.setBranchResultPoints( branchResultPoints );
                } // End of the branch loop

                // Set modified copy back to frame
                wellResFrame.setWellResultBranches( wellResultBranches );

                // Propagate position contributions from connections above unpositioned segments downwards

                well_segment_collection_type* allErtSegments = well_state_get_segments( ert_well_state );

                bool isWellHead = true;
                for ( const auto& wellResultBranch : wellResFrame.wellResultBranches() )
                {
                    bool previousResultPointWasCell = isWellHead;

                    // Go downwards until we find a none-cell result point just after a cell result point
                    // When we do, start propagating

                    for ( size_t rpIdx = 0; rpIdx < wellResultBranch.branchResultPoints().size(); ++rpIdx )
                    {
                        const RigWellResultPoint resPoint = wellResultBranch.branchResultPoints()[rpIdx];
                        if ( resPoint.isCell() )
                        {
                            previousResultPointWasCell = true;
                        }
                        else
                        {
                            if ( previousResultPointWasCell )
                            {
                                RigWellResultPoint prevResPoint;
                                if ( isWellHead && rpIdx == 0 )
                                {
                                    prevResPoint = wellResFrame.wellHead();
                                }
                                else
                                {
                                    prevResPoint = wellResultBranch.branchResultPoints()[rpIdx - 1];
                                }

                                cvf::Vec3d lastConnectionPos = grids[prevResPoint.gridIndex()]->cell( prevResPoint.cellIndex() ).center();

                                SegmentPositionContribution
                                    posContrib( prevResPoint.segmentId(), lastConnectionPos, 0.0, false, -1, prevResPoint.segmentId(), true );

                                int ertSegmentId = resPoint.segmentId();

                                std::map<int, std::vector<SegmentPositionContribution>>::iterator posContribIt;
                                posContribIt = segmentIdToPositionContrib.find( ertSegmentId );
                                CVF_ASSERT( posContribIt != segmentIdToPositionContrib.end() );

                                std::vector<SegmentPositionContribution> posContributions = posContribIt->second;
                                for ( size_t i = 0; i < posContributions.size(); ++i )
                                {
                                    posContributions[i].m_segmentIdAbove = prevResPoint.segmentId();
                                }
                                posContributions.push_back( posContrib );

                                propagatePosContribDownwards( segmentIdToPositionContrib, allErtSegments, ertSegmentId, posContributions );
                            }

                            previousResultPointWasCell = false;
                        }
                    }

                    isWellHead = false;
                }

                // Calculate the bottom position of all the unpositioned segments
                // Then do the calculation based on the refined contributions

                std::map<int, std::vector<SegmentPositionContribution>>::iterator posContribIt = segmentIdToPositionContrib.begin();
                std::map<int, cvf::Vec3d>                                         bottomPositions;
                while ( posContribIt != segmentIdToPositionContrib.end() )
                {
                    bottomPositions[posContribIt->first] = interpolate3DPosition( posContribIt->second );
                    ++posContribIt;
                }

                // Copy content and distribute the positions to the result points stored in the wellResultBranch.branchResultPoints()
                // set updated copy back to frame

                std::vector<RigWellResultBranch> newWellResultBranches = wellResFrame.wellResultBranches();
                for ( auto& wellResultBranch : newWellResultBranches )
                {
                    RigWellResultBranch& newWellResultBranch = wellResultBranch;
                    for ( auto& resultPoint : newWellResultBranch.branchResultPoints() )
                    {
                        if ( !resultPoint.isCell() )
                        {
                            resultPoint.setBottomPosition( bottomPositions[resultPoint.segmentId()] );
                        }
                    }
                }
                wellResFrame.setWellResultBranches( newWellResultBranches );
            } // End of the MSW section
            else
            {
                // Code handling None-MSW Wells ... Normal wells that is.

                WellResultPointHasSubCellConnectionCalculator subCellConnCalc( m_eclipseCase->mainGrid(), ert_well_state );
                int                                           lastGridNr = static_cast<int>( grids.size() ) - 1;
                for ( int gridNr = 0; gridNr <= lastGridNr; ++gridNr )
                {
                    const well_conn_type* ert_wellhead = well_state_iget_wellhead( ert_well_state, static_cast<int>( gridNr ) );
                    if ( ert_wellhead )
                    {
                        RigWellResultPoint wellHeadRp = createWellResultPoint( grids[gridNr], ert_wellhead, wellName );
                        // HACK: Ert returns open as "this is equally wrong as closed for well heads".
                        // Well heads are not open jfr mail communication with HHGS and JH Statoil 07.01.2016
                        wellHeadRp.setIsOpen( false );

                        if ( !subCellConnCalc.hasSubCellConnection( wellHeadRp ) ) wellResFrame.setWellHead( wellHeadRp );
                    }

                    const well_conn_collection_type* connections =
                        well_state_get_grid_connections( ert_well_state, ertGridName( gridNr ).data() );

                    // Import all well result cells for all connections
                    if ( connections )
                    {
                        int connectionCount = well_conn_collection_get_size( connections );
                        if ( connectionCount )
                        {
                            RigWellResultBranch wellResultBranch;
                            wellResultBranch.setErtBranchId( 0 ); // Normal wells have only one branch

                            std::vector<RigWellResultPoint> branchResultPoints = wellResultBranch.branchResultPoints();
                            const size_t                    existingCellCount  = branchResultPoints.size();
                            branchResultPoints.resize( existingCellCount + connectionCount );

                            for ( int connIdx = 0; connIdx < connectionCount; connIdx++ )
                            {
                                well_conn_type*    ert_connection = well_conn_collection_iget( connections, connIdx );
                                RigWellResultPoint wellRp         = createWellResultPoint( grids[gridNr], ert_connection, wellName );

                                if ( !subCellConnCalc.hasSubCellConnection( wellRp ) )
                                {
                                    branchResultPoints[existingCellCount + connIdx] = wellRp;
                                }
                            }
                            wellResultBranch.setBranchResultPoints( branchResultPoints );
                            wellResFrame.addWellResultBranch( wellResultBranch );
                        }
                    }
                }
            }
        }

        std::vector<QDateTime> filteredTimeSteps;
        {
            std::vector<RigEclipseTimeStepInfo> filteredTimeStepInfos = createFilteredTimeStepInfos();
            for ( auto a : filteredTimeStepInfos )
            {
                filteredTimeSteps.push_back( a.m_date );
            }
        }

        simWellData->computeMappingFromResultTimeIndicesToWellTimeIndices( filteredTimeSteps );

        wells.push_back( simWellData.p() );

        progress.incrementProgress();
    }

    well_info_free( ert_well_info );

    m_eclipseCase->setSimWellData( wells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifKeywordValueCount>
    RifReaderEclipseOutput::validKeywordsForPorosityModel( const std::vector<RifKeywordValueCount>& keywordItemCounts,
                                                           const RigActiveCellInfo*                 matrixActiveCellInfo,
                                                           const RigActiveCellInfo*                 fractureActiveCellInfo,
                                                           RiaDefines::PorosityModelType            porosityModel,
                                                           size_t                                   timeStepCount )
{
    CVF_ASSERT( matrixActiveCellInfo );

    if ( porosityModel == RiaDefines::PorosityModelType::FRACTURE_MODEL )
    {
        if ( fractureActiveCellInfo->reservoirActiveCellCount() == 0 )
        {
            return {};
        }
    }

    std::vector<RifKeywordValueCount> keywordsWithCorrectNumberOfDataItems;

    for ( const auto& keywordValueCount : keywordItemCounts )
    {
        QString keyword    = QString::fromStdString( keywordValueCount.keyword() );
        size_t  valueCount = keywordValueCount.valueCount();

        bool validKeyword = false;

        size_t timeStepsAllCellsRest = valueCount % matrixActiveCellInfo->reservoirCellCount();
        if ( timeStepsAllCellsRest == 0 && valueCount <= timeStepCount * matrixActiveCellInfo->reservoirCellCount() )
        {
            // Found result for all cells for N time steps, usually a static dataset for one time step
            validKeyword = true;
        }
        else
        {
            size_t timeStepsMatrixRest = valueCount % matrixActiveCellInfo->reservoirActiveCellCount();

            size_t timeStepsFractureRest = 0;
            if ( fractureActiveCellInfo->reservoirActiveCellCount() > 0 )
            {
                timeStepsFractureRest = valueCount % fractureActiveCellInfo->reservoirActiveCellCount();
            }

            size_t sumFractureMatrixActiveCellCount = matrixActiveCellInfo->reservoirActiveCellCount() +
                                                      fractureActiveCellInfo->reservoirActiveCellCount();
            size_t timeStepsMatrixAndFractureRest = valueCount % sumFractureMatrixActiveCellCount;

            if ( porosityModel == RiaDefines::PorosityModelType::MATRIX_MODEL && timeStepsMatrixRest == 0 )
            {
                if ( valueCount <=
                     timeStepCount * std::max( matrixActiveCellInfo->reservoirActiveCellCount(), sumFractureMatrixActiveCellCount ) )
                {
                    validKeyword = true;
                }
            }
            else if ( porosityModel == RiaDefines::PorosityModelType::FRACTURE_MODEL &&
                      fractureActiveCellInfo->reservoirActiveCellCount() > 0 && timeStepsFractureRest == 0 )
            {
                if ( valueCount <=
                     timeStepCount * std::max( fractureActiveCellInfo->reservoirActiveCellCount(), sumFractureMatrixActiveCellCount ) )
                {
                    validKeyword = true;
                }
            }
            else if ( timeStepsMatrixAndFractureRest == 0 )
            {
                if ( valueCount <= timeStepCount * sumFractureMatrixActiveCellCount )
                {
                    validKeyword = true;
                }
            }
        }

        // Check for INIT values that has only values for main grid active cells
        if ( !validKeyword )
        {
            if ( timeStepCount == 1 )
            {
                size_t mainGridMatrixActiveCellCount   = matrixActiveCellInfo->gridActiveCellCounts( 0 );
                size_t mainGridFractureActiveCellCount = fractureActiveCellInfo->gridActiveCellCounts( 0 );

                if ( valueCount == mainGridMatrixActiveCellCount || valueCount == mainGridFractureActiveCellCount ||
                     valueCount == mainGridMatrixActiveCellCount + mainGridFractureActiveCellCount )
                {
                    validKeyword = true;
                }
            }
        }

        if ( validKeyword )
        {
            keywordsWithCorrectNumberOfDataItems.push_back( keywordValueCount );
        }
    }

    return keywordsWithCorrectNumberOfDataItems;
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
void RifReaderEclipseOutput::extractResultValuesBasedOnPorosityModel( RiaDefines::PorosityModelType matrixOrFracture,
                                                                      std::vector<double>*          destinationResultValues,
                                                                      const std::vector<double>&    sourceResultValues )
{
    if ( sourceResultValues.empty() ) return;

    RigActiveCellInfo* fracActCellInfo = m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    if ( matrixOrFracture == RiaDefines::PorosityModelType::MATRIX_MODEL && fracActCellInfo->reservoirActiveCellCount() == 0 )
    {
        destinationResultValues->insert( destinationResultValues->end(), sourceResultValues.begin(), sourceResultValues.end() );
    }
    else
    {
        RigActiveCellInfo* actCellInfo = m_eclipseCase->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

        size_t sourceStartPosition = 0;

        for ( size_t i = 0; i < m_eclipseCase->mainGrid()->gridCount(); i++ )
        {
            if ( m_eclipseCase->mainGrid()->gridByIndex( i )->isTempGrid() ) continue;

            size_t matrixActiveCellCount   = actCellInfo->gridActiveCellCounts( i );
            size_t fractureActiveCellCount = fracActCellInfo->gridActiveCellCounts( i );

            if ( matrixOrFracture == RiaDefines::PorosityModelType::MATRIX_MODEL )
            {
                destinationResultValues->insert( destinationResultValues->end(),
                                                 sourceResultValues.begin() + sourceStartPosition,
                                                 sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount );
            }
            else
            {
                if ( ( matrixActiveCellCount + fractureActiveCellCount ) > sourceResultValues.size() )
                {
                    // Special handling of the situation where we only have data for one fracture mode
                    matrixActiveCellCount = 0;
                }

                destinationResultValues->insert( destinationResultValues->end(),
                                                 sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount,
                                                 sourceResultValues.begin() + sourceStartPosition + matrixActiveCellCount +
                                                     fractureActiveCellCount );
            }

            sourceStartPosition += ( matrixActiveCellCount + fractureActiveCellCount );
        }
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

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::string RifReaderEclipseOutput::ertGridName( size_t gridNr )
{
    std::string gridName;
    if ( gridNr == 0 )
    {
        gridName = ECL_GRID_GLOBAL_GRID;
    }
    else
    {
        CVF_ASSERT( m_eclipseCase );
        CVF_ASSERT( m_eclipseCase->gridCount() > gridNr );
        gridName = m_eclipseCase->grid( gridNr )->gridName();
    }

    return gridName;
}
