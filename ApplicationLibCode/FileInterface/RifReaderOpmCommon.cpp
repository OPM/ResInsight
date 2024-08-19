/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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

#include "RifReaderOpmCommon.h"

#include "RiaEclipseFileNameTools.h"
#include "RiaLogging.h"
#include "RiaQDateTimeTools.h"
#include "RiaStdStringTools.h"

#include "RifEclipseOutputFileTools.h"
#include "RifEclipseReportKeywords.h"
#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifOpmRadialGridTools.h"
#include "RifReaderEclipseWell.h"

#include "RigActiveCellGrid.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "RigSimWellData.h"
#include "RigWellResultFrame.h"

#include "cafProgressInfo.h"

#include "opm/io/eclipse/EGrid.hpp"
#include "opm/io/eclipse/EInit.hpp"
#include "opm/io/eclipse/ERst.hpp"
#include "opm/output/eclipse/VectorItems/intehead.hpp"

#include <QStringList>

using namespace Opm;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommon::RifReaderOpmCommon()
    : m_eclipseCaseData( nullptr )
    , m_gridUnit( -1 )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommon::~RifReaderOpmCommon()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::open( const QString& fileName, RigEclipseCaseData* eclipseCaseData )
{
    caf::ProgressInfo progress( 100, "Reading Grid" );

    QStringList fileSet;
    if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &fileSet ) ) return false;

    try
    {
        m_gridFileName = fileName.toStdString();
        locateInitAndRestartFiles( fileName );

        if ( onlyLoadActiveCells() )
        {
            auto mainGrid = new RigActiveCellGrid();
            eclipseCaseData->setMainGrid( mainGrid );
            if ( !importActiveGrid( mainGrid, eclipseCaseData ) )
            {
                RiaLogging::error( "Failed to open grid file " + fileName );

                return false;
            }
        }
        else
        {
            if ( !importGrid( eclipseCaseData->mainGrid(), eclipseCaseData ) )
            {
                RiaLogging::error( "Failed to open grid file " + fileName );

                return false;
            }
        }

        {
            auto task = progress.task( "Reading faults", 25 );

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
            buildMetaData( eclipseCaseData, progress );
        }

        auto task = progress.task( "Handling NCC Result data", 25 );
        if ( isNNCsEnabled() )
        {
            caf::ProgressInfo nncProgress( 10, "" );
            RigMainGrid*      mainGrid = eclipseCaseData->mainGrid();

            // This test should probably be improved to test more directly for presence of NNC data
            if ( eclipseCaseData->results( RiaDefines::PorosityModelType::MATRIX_MODEL )->hasFlowDiagUsableFluxes() )
            {
                auto subNncTask = nncProgress.task( "Reading dynamic NNC data" );
                transferDynamicNNCData( mainGrid );
            }

            RigActiveCellInfo* activeCellInfo = m_eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );

            bool includeInactiveCells = includeInactiveCellsInFaultGeometry();
            mainGrid->nncData()->setSourceDataForProcessing( mainGrid, activeCellInfo, includeInactiveCells );
        }

        return true;
    }
    catch ( std::exception& e )
    {
        auto description = e.what();
        RiaLogging::error( description );
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::importGrid( RigMainGrid* mainGrid, RigEclipseCaseData* eclipseCaseData )
{
    caf::ProgressInfo progInfo( 5, "Importing Eclipse Grid" );

    Opm::EclIO::EGrid opmGrid( m_gridFileName );

    RigActiveCellInfo* activeCellInfo         = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    const auto& dims = opmGrid.dimension();
    mainGrid->setGridPointDimensions( cvf::Vec3st( dims[0] + 1, dims[1] + 1, dims[2] + 1 ) );
    mainGrid->setGridName( "Main grid" );
    mainGrid->setDualPorosity( opmGrid.porosity_mode() > 0 );

    // assign grid unit, if found (1 = Metric, 2 = Field, 3 = Lab)
    auto gridUnitStr = RiaStdStringTools::toUpper( opmGrid.grid_unit() );
    if ( gridUnitStr.starts_with( 'M' ) )
        m_gridUnit = 1;
    else if ( gridUnitStr.starts_with( 'F' ) )
        m_gridUnit = 2;
    else if ( gridUnitStr.starts_with( 'C' ) )
        m_gridUnit = 3;

    auto totalCellCount           = opmGrid.totalNumberOfCells();
    auto globalMatrixActiveSize   = opmGrid.activeCells();
    auto globalFractureActiveSize = opmGrid.activeFracCells();

    const auto& lgr_names = opmGrid.list_of_lgrs();
    m_gridNames.clear();
    m_gridNames.push_back( "global" );
    m_gridNames.insert( m_gridNames.end(), lgr_names.begin(), lgr_names.end() );
    const auto& lgr_parent_names = opmGrid.list_of_lgr_parents();
    const int   numLGRs          = (int)lgr_names.size();

    std::vector<Opm::EclIO::EGrid> lgrGrids;

    for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
    {
        lgrGrids.emplace_back( Opm::EclIO::EGrid( m_gridFileName, lgr_names[lgrIdx] ) );
        RigLocalGrid* localGrid = new RigLocalGrid( mainGrid );

        const auto& lgrDims = lgrGrids[lgrIdx].dimension();
        localGrid->setGridPointDimensions( cvf::Vec3st( lgrDims[0] + 1, lgrDims[1] + 1, lgrDims[2] + 1 ) );

        localGrid->setGridId( lgrIdx + 1 );
        localGrid->setGridName( lgr_names[lgrIdx] );
        mainGrid->addLocalGrid( localGrid );

        localGrid->setIndexToStartOfCells( totalCellCount );

        totalCellCount += lgrGrids[lgrIdx].totalNumberOfCells();
    }

    activeCellInfo->setReservoirCellCount( totalCellCount );
    fractureActiveCellInfo->setReservoirCellCount( totalCellCount );

    mainGrid->globalCellArray().reserve( (size_t)totalCellCount );
    mainGrid->nodes().reserve( (size_t)totalCellCount * 8 );

    activeCellInfo->setGridCount( 1 + numLGRs );
    fractureActiveCellInfo->setGridCount( 1 + numLGRs );

    {
        auto task = progInfo.task( "Getting Active Cell Information", 1 );

        for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
        {
            globalMatrixActiveSize += lgrGrids[lgrIdx].activeCells();
            globalFractureActiveSize += lgrGrids[lgrIdx].activeFracCells();
        }

        // in case init file and grid file disagrees with number of active cells, read extra porv information from init file to correct this
        if ( !verifyActiveCellInfo( globalMatrixActiveSize, globalFractureActiveSize ) )
        {
            updateActiveCellInfo( eclipseCaseData, opmGrid, lgrGrids, mainGrid );
        }

        globalMatrixActiveSize   = opmGrid.activeCells();
        globalFractureActiveSize = opmGrid.activeFracCells();

        activeCellInfo->setGridActiveCellCounts( 0, globalMatrixActiveSize );
        fractureActiveCellInfo->setGridActiveCellCounts( 0, globalFractureActiveSize );

        transferActiveCells( opmGrid, 0, eclipseCaseData, 0, 0 );
        size_t cellCount = opmGrid.totalNumberOfCells();

        for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
        {
            auto& lgrGrid = lgrGrids[lgrIdx];
            transferActiveCells( lgrGrid, cellCount, eclipseCaseData, globalMatrixActiveSize, globalFractureActiveSize );
            cellCount += lgrGrid.totalNumberOfCells();
            globalMatrixActiveSize += lgrGrid.activeCells();
            globalFractureActiveSize += lgrGrid.activeFracCells();
            activeCellInfo->setGridActiveCellCounts( lgrIdx + 1, lgrGrid.activeCells() );
            fractureActiveCellInfo->setGridActiveCellCounts( lgrIdx + 1, lgrGrid.activeFracCells() );
        }

        activeCellInfo->computeDerivedData();
        fractureActiveCellInfo->computeDerivedData();
    }

    {
        auto task = progInfo.task( "Loading Main Grid Geometry", 1 );
        transferGeometry( opmGrid, opmGrid, mainGrid, mainGrid, eclipseCaseData );
    }

    bool hasParentInfo = ( lgr_parent_names.size() >= (size_t)numLGRs );

    auto task = progInfo.task( "Loading LGR Grid Geometry ", 1 );

    for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
    {
        RigGridBase* parentGrid = hasParentInfo ? mainGrid->gridByName( lgr_parent_names[lgrIdx] ) : mainGrid;

        RigLocalGrid* localGrid = static_cast<RigLocalGrid*>( mainGrid->gridById( lgrIdx + 1 ) );
        localGrid->setParentGrid( parentGrid );

        transferGeometry( opmGrid, lgrGrids[lgrIdx], mainGrid, localGrid, eclipseCaseData );
    }

    mainGrid->initAllSubGridsParentGridPointer();

    if ( isNNCsEnabled() )
    {
        auto task = progInfo.task( "Loading NNC data", 1 );
        transferStaticNNCData( opmGrid, lgrGrids, mainGrid );
    }

    auto opmMapAxes = opmGrid.get_mapaxes();
    if ( opmMapAxes.size() == 6 )
    {
        std::array<double, 6> mapAxes;
        for ( size_t i = 0; i < opmMapAxes.size(); ++i )
        {
            mapAxes[i] = opmMapAxes[i];
        }

        // Set the map axes transformation matrix on the main grid
        mainGrid->setMapAxes( mapAxes );
        mainGrid->setUseMapAxes( true );

        auto transform = mainGrid->mapAxisTransform();

        // Invert the transformation matrix to convert from file coordinates to domain coordinates
        transform.invert();

#pragma omp parallel for
        for ( long i = 0; i < static_cast<long>( mainGrid->nodes().size() ); i++ )
        {
            auto& n = mainGrid->nodes()[i];
            n.transformPoint( transform );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::transferStaticNNCData( Opm::EclIO::EGrid& opmMainGrid, std::vector<Opm::EclIO::EGrid>& lgrGrids, RigMainGrid* mainGrid )
{
    opmMainGrid.load_nnc_data();
    for ( auto& lgr : lgrGrids )
    {
        lgr.load_nnc_data();
    }

    auto connections = opmMainGrid.nnc_connections( 0 );

    for ( int i = 0; i < (int)lgrGrids.size(); i++ )
    {
        auto conn = lgrGrids[i].nnc_connections( i + 1 );
        connections.insert( connections.end(), conn.begin(), conn.end() );
    }

    if ( !connections.empty() )
    {
        // Transform to our own data structures
        RigConnectionContainer nncConnections;
        std::vector<double>    transmissibilityValues;

        for ( auto& c : connections )
        {
            RigGridBase* grid1 = mainGrid->gridByIndex( c.grid1_Id );
            RigGridBase* grid2 = mainGrid->gridByIndex( c.grid2_Id );

            RigConnection nncConnection( grid1->reservoirCellIndex( c.grid1_CellIdx - 1 ), grid2->reservoirCellIndex( c.grid2_CellIdx - 1 ) );

            nncConnections.push_back( nncConnection );

            transmissibilityValues.push_back( c.transValue );
        }

        mainGrid->nncData()->setEclipseConnections( nncConnections );
        mainGrid->nncData()->makeScalarResultAndSetValues( RiaDefines::propertyNameCombTrans(), transmissibilityValues );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::transferDynamicNNCData( RigMainGrid* mainGrid )
{
    if ( !m_restartFile ) return;

    const size_t timeStepCount = m_restartFile->numberOfReportSteps();

    std::vector<std::vector<double>>& waterFluxData =
        mainGrid->nncData()->makeDynamicConnectionScalarResult( RiaDefines::propertyNameFluxWat(), timeStepCount );
    std::vector<std::vector<double>>& oilFluxData =
        mainGrid->nncData()->makeDynamicConnectionScalarResult( RiaDefines::propertyNameFluxOil(), timeStepCount );
    std::vector<std::vector<double>>& gasFluxData =
        mainGrid->nncData()->makeDynamicConnectionScalarResult( RiaDefines::propertyNameFluxGas(), timeStepCount );

    for ( size_t timeStep = 0; timeStep < timeStepCount; ++timeStep )
    {
        dynamicResult( "FLRWATN+", RiaDefines::PorosityModelType::MATRIX_MODEL, timeStep, &waterFluxData[timeStep] );
        dynamicResult( "FLRGASN+", RiaDefines::PorosityModelType::MATRIX_MODEL, timeStep, &gasFluxData[timeStep] );
        dynamicResult( "FLROILN+", RiaDefines::PorosityModelType::MATRIX_MODEL, timeStep, &oilFluxData[timeStep] );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::transferActiveCells( Opm::EclIO::EGrid&  opmGrid,
                                              size_t              cellStartIndex,
                                              RigEclipseCaseData* eclipseCaseData,
                                              size_t              matrixActiveStartIndex,
                                              size_t              fractureActiveStartIndex )
{
    const int cellCount = opmGrid.totalNumberOfCells();

    RigActiveCellInfo* activeCellInfo         = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    const auto& active_indexes      = opmGrid.active_indexes();
    const auto& active_frac_indexes = opmGrid.active_frac_indexes();

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < (int)cellCount; opmCellIndex++ )
    {
        // active cell index
        int matrixActiveIndex = active_indexes[opmCellIndex];
        if ( matrixActiveIndex != -1 )
        {
            activeCellInfo->setCellResultIndex( cellStartIndex + opmCellIndex, matrixActiveStartIndex + matrixActiveIndex );
        }

        int fractureActiveIndex = active_frac_indexes[opmCellIndex];
        if ( fractureActiveIndex != -1 )
        {
            fractureActiveCellInfo->setCellResultIndex( cellStartIndex + opmCellIndex, fractureActiveStartIndex + fractureActiveIndex );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::transferGeometry( Opm::EclIO::EGrid&  opmMainGrid,
                                           Opm::EclIO::EGrid&  opmGrid,
                                           RigMainGrid*        mainGrid,
                                           RigGridBase*        localGrid,
                                           RigEclipseCaseData* eclipseCaseData )
{
    int    cellCount      = opmGrid.totalNumberOfCells();
    size_t cellStartIndex = mainGrid->globalCellArray().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigCell defaultCell;
    defaultCell.setHostGrid( localGrid );

    mainGrid->globalCellArray().resize( cellStartIndex + cellCount, defaultCell );

    mainGrid->nodes().resize( nodeStartIndex + cellCount * 8, cvf::Vec3d( 0, 0, 0 ) );

    auto& riNodes = mainGrid->nodes();

    opmGrid.loadData();
    opmGrid.load_grid_data();

    const bool isRadialGrid = opmGrid.is_radial();

    const auto& gridDimension         = opmGrid.dimension();
    const auto& hostCellGlobalIndices = opmGrid.hostCellsGlobalIndex();

    // Compute the center of the LGR radial grid cells for each K layer
    auto radialGridCenterTopLayerOpm = isRadialGrid ? RifOpmRadialGridTools::computeXyCenterForTopOfCells( opmMainGrid, opmGrid, localGrid )
                                                    : std::map<int, std::pair<double, double>>();

    // use same mapping as resdata
    const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( localGrid->cellCount() ); opmCellIndex++ )
    {
        auto opmIJK = opmGrid.ijk_from_global_index( opmCellIndex );

        double xCenterCoordOpm = 0.0;
        double yCenterCoordOpm = 0.0;

        if ( isRadialGrid && radialGridCenterTopLayerOpm.contains( opmIJK[2] ) )
        {
            const auto& [xCenter, yCenter] = radialGridCenterTopLayerOpm[opmIJK[2]];
            xCenterCoordOpm                = xCenter;
            yCenterCoordOpm                = yCenter;
        }

        auto     riReservoirIndex = localGrid->cellIndexFromIJK( opmIJK[0], opmIJK[1], opmIJK[2] );
        RigCell& cell             = mainGrid->globalCellArray()[cellStartIndex + riReservoirIndex];
        cell.setGridLocalCellIndex( riReservoirIndex );

        // parent cell index
        if ( ( hostCellGlobalIndices.size() > (size_t)opmCellIndex ) && hostCellGlobalIndices[opmCellIndex] >= 0 )
        {
            cell.setParentCellIndex( hostCellGlobalIndices[opmCellIndex] );
        }
        else
        {
            cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );
        }

        // corner coordinates
        std::array<double, 8> opmX{};
        std::array<double, 8> opmY{};
        std::array<double, 8> opmZ{};
        opmGrid.getCellCorners( opmCellIndex, opmX, opmY, opmZ );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto riNodeStartIndex = nodeStartIndex + riReservoirIndex * 8;

        for ( size_t opmNodeIndex = 0; opmNodeIndex < 8; opmNodeIndex++ )
        {
            auto   riCornerIndex = cellMappingECLRi[opmNodeIndex];
            size_t riNodeIndex   = riNodeStartIndex + riCornerIndex;

            auto& riNode = riNodes[riNodeIndex];
            riNode.x()   = opmX[opmNodeIndex] + xCenterCoordOpm;
            riNode.y()   = opmY[opmNodeIndex] + yCenterCoordOpm;
            riNode.z()   = -opmZ[opmNodeIndex];

            cell.cornerIndices()[riCornerIndex] = riNodeIndex;

            // First grid dimension is radius, check if cell are at the outer-most slice
            if ( isRadialGrid && !hostCellGlobalIndices.empty() && ( gridDimension[0] - 1 == opmIJK[0] ) )
            {
                auto hostCellIndex = hostCellGlobalIndices[opmCellIndex];

                RifOpmRadialGridTools::lockToHostPillars( riNode,
                                                          opmMainGrid,
                                                          opmGrid,
                                                          opmIJK,
                                                          hostCellIndex,
                                                          opmCellIndex,
                                                          opmNodeIndex,
                                                          xCenterCoordOpm,
                                                          yCenterCoordOpm );
            }
        }

        cell.setInvalid( cell.isLongPyramidCell() );
    }

    // subgrid pointers
    RigLocalGrid* realLocalGrid = dynamic_cast<RigLocalGrid*>( localGrid );
    RigGridBase*  parentGrid    = realLocalGrid != nullptr ? realLocalGrid->parentGrid() : nullptr;

    if ( parentGrid != nullptr )
    {
        for ( auto localCellInGlobalIdx : hostCellGlobalIndices )
        {
            parentGrid->cell( localCellInGlobalIdx ).setSubGrid( realLocalGrid );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    if ( m_initFile )
    {
        try
        {
            auto resultName = result.toStdString();

            std::vector<double> combinedFileValues;

            const auto& resultEntries = m_initFile->getList();
            for ( const auto& entry : resultEntries )
            {
                const auto& [keyword, kwType, size] = entry;
                if ( keyword == resultName )
                {
                    for ( auto& gridName : m_gridNames )
                    {
                        if ( kwType == EclIO::eclArrType::REAL )
                        {
                            const auto& fileValues = m_initFile->getInitData<float>( resultName, gridName );
                            combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                        }
                        else if ( kwType == EclIO::eclArrType::DOUB )
                        {
                            const auto& fileValues = m_initFile->getInitData<double>( resultName, gridName );
                            combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                        }
                        else if ( kwType == EclIO::eclArrType::INTE )
                        {
                            const auto& fileValues = m_initFile->getInitData<int>( resultName, gridName );
                            combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                        }
                    }
                    break;
                }
            }

            // Always clear data after reading to avoid memory use
            m_initFile->clearData();

            RifEclipseOutputFileTools::extractResultValuesBasedOnPorosityModel( m_eclipseCaseData, matrixOrFracture, values, combinedFileValues );

            return true;
        }
        catch ( std::exception& e )
        {
            RiaLogging::error( e.what() );
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::dynamicResult( const QString&                result,
                                        RiaDefines::PorosityModelType matrixOrFracture,
                                        size_t                        stepIndex,
                                        std::vector<double>*          values )
{
    if ( m_restartFile )
    {
        try
        {
            auto resultName = result.toStdString();

            size_t indexOnFile = timeStepIndexOnFile( stepIndex );

            const auto& stepNumbers = m_restartFile->listOfReportStepNumbers();
            auto        stepNumber  = stepNumbers[indexOnFile];

            std::vector<double> combinedFileValues;

            auto resultEntries = m_restartFile->getList();
            for ( const auto& entry : resultEntries )
            {
                const auto& [keyword, kwType, size] = entry;
                if ( keyword == resultName )
                {
                    for ( auto& gridName : m_gridNames )
                    {
                        if ( gridName == "global" ) // main grid, need to use separate method due to inner workings in opm_common
                        {
                            if ( kwType == EclIO::eclArrType::DOUB )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<double>( resultName, stepNumber );
                                combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                            }
                            if ( kwType == EclIO::eclArrType::REAL )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<float>( resultName, stepNumber );
                                combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                            }
                            else if ( kwType == EclIO::eclArrType::INTE )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<int>( resultName, stepNumber );
                                combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                            }
                        }
                        else
                        {
                            if ( kwType == EclIO::eclArrType::DOUB )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<double>( resultName, stepNumber, gridName );
                                combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                            }
                            if ( kwType == EclIO::eclArrType::REAL )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<float>( resultName, stepNumber, gridName );
                                combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                            }
                            else if ( kwType == EclIO::eclArrType::INTE )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<int>( resultName, stepNumber, gridName );
                                combinedFileValues.insert( combinedFileValues.end(), fileValues.begin(), fileValues.end() );
                            }
                        }
                    }
                    break;
                }
            }

            // Always clear data after reading to avoid memory use
            m_restartFile->clearData();

            RifEclipseOutputFileTools::extractResultValuesBasedOnPorosityModel( m_eclipseCaseData, matrixOrFracture, values, combinedFileValues );

            return true;
        }
        catch ( std::exception& e )
        {
            RiaLogging::error( e.what() );
        }
    }

    return false;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static std::vector<RifEclipseKeywordValueCount> createKeywordInfo( std::vector<EclIO::EclFile::EclEntry> entries )
{
    RifEclipseReportKeywords keywordsReport;

    for ( const auto& entry : entries )
    {
        const auto& [keyword, kwType, size] = entry;

        RifEclipseKeywordValueCount::KeywordDataType dataType = RifEclipseKeywordValueCount::KeywordDataType::UNKNOWN;

        if ( kwType == EclIO::eclArrType::INTE )
            dataType = RifEclipseKeywordValueCount::KeywordDataType::INTEGER;
        else if ( kwType == EclIO::eclArrType::REAL )
            dataType = RifEclipseKeywordValueCount::KeywordDataType::FLOAT;
        else if ( kwType == EclIO::eclArrType::DOUB )
            dataType = RifEclipseKeywordValueCount::KeywordDataType::DOUBLE;

        if ( dataType != RifEclipseKeywordValueCount::KeywordDataType::UNKNOWN )
        {
            keywordsReport.appendKeywordCount( keyword, static_cast<size_t>( size ), dataType );
        }
    }

    return keywordsReport.keywordValueCounts();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::locateInitAndRestartFiles( QString gridFileName )
{
    auto getFileNameForType = []( RiaEclipseFileNameTools::EclipseFileType fileType, const QString& candidate ) -> std::string
    {
        const QString ext = caf::AppEnum<RiaEclipseFileNameTools::EclipseFileType>::text( fileType );
        if ( candidate.endsWith( ext, Qt::CaseInsensitive ) ) return candidate.toStdString();
        return {};
    };

    if ( m_initFileName.empty() || m_restartFileName.empty() )
    {
        QStringList fileSet;
        RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( gridFileName, &fileSet );
        for ( const auto& s : fileSet )
        {
            auto initCandidate    = getFileNameForType( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_INIT, s );
            auto restartCandidate = getFileNameForType( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_UNRST, s );

            if ( !initCandidate.empty() ) m_initFileName = initCandidate;
            if ( !restartCandidate.empty() ) m_restartFileName = restartCandidate;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::setupInitAndRestartAccess()
{
    if ( ( m_initFile == nullptr ) && !m_initFileName.empty() )
    {
        m_initFile = std::make_unique<EclIO::EInit>( m_initFileName );
    }

    if ( ( m_restartFile == nullptr ) && !m_restartFileName.empty() )
    {
        m_restartFile = std::make_unique<EclIO::ERst>( m_restartFileName );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RigEclipseTimeStepInfo> RifReaderOpmCommon::createFilteredTimeStepInfos()
{
    std::vector<RigEclipseTimeStepInfo> timeStepInfos;

    if ( m_restartFile == nullptr ) return timeStepInfos;

    auto timeStepsOnFile = readTimeSteps();

    for ( size_t i = 0; i < timeStepsOnFile.size(); i++ )
    {
        if ( isTimeStepIncludedByFilter( i ) )
        {
            QDate date( timeStepsOnFile[i].year, timeStepsOnFile[i].month, timeStepsOnFile[i].day );
            auto  datetime = RiaQDateTimeTools::createDateTime( date, Qt::TimeSpec::UTC );

            timeStepInfos.push_back(
                RigEclipseTimeStepInfo( datetime, timeStepsOnFile[i].sequenceNumber, timeStepsOnFile[i].simulationTimeFromStart ) );
        }
    }

    return timeStepInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::buildMetaData( RigEclipseCaseData* eclipseCaseData, caf::ProgressInfo& progress )
{
    setupInitAndRestartAccess();

    std::vector<QDateTime>              timeSteps;
    std::vector<RigEclipseTimeStepInfo> filteredTimeStepInfos;

    RigEclipseTimeStepInfo firstTimeStepInfo{ QDateTime(), 0, 0.0 };
    if ( m_restartFile != nullptr )
    {
        std::vector<EclIO::EclFile::EclEntry> entries;
        for ( auto reportNumber : m_restartFile->listOfReportStepNumbers() )
        {
            auto stepEntries = m_restartFile->listOfRstArrays( reportNumber );

            std::set<std::pair<std::string, Opm::EclIO::eclArrType>> keyNames;

            for ( auto& [keyName, resType, nValues] : stepEntries )
            {
                keyNames.insert( { keyName, resType } );
            }

            for ( auto& [keyName, resType] : keyNames )
            {
                auto dataSize = m_restartFile->dataSize( keyName, reportNumber );

                entries.emplace_back( keyName, resType, dataSize );
            }
        }

        auto last = std::unique( entries.begin(), entries.end() );
        entries.erase( last, entries.end() );

        std::vector<RifEclipseKeywordValueCount> keywordInfo = createKeywordInfo( entries );

        filteredTimeStepInfos = createFilteredTimeStepInfos();

        RifEclipseOutputFileTools::createResultEntries( keywordInfo,
                                                        filteredTimeStepInfos,
                                                        RiaDefines::ResultCatType::DYNAMIC_NATIVE,
                                                        eclipseCaseData,
                                                        m_restartFile->numberOfReportSteps() );

        if ( !filteredTimeStepInfos.empty() ) firstTimeStepInfo = filteredTimeStepInfos.front();
    }

    if ( m_initFile != nullptr )
    {
        // entries from main grid
        auto entries = m_initFile->list_arrays();
        // add lgr entries, too
        auto nGrids = m_gridNames.size();
        for ( size_t i = 1; i < nGrids; i++ )
        {
            auto gridEntries = m_initFile->list_arrays( m_gridNames[i] );
            entries.insert( entries.end(), gridEntries.begin(), gridEntries.end() );
        }

        std::vector<RifEclipseKeywordValueCount> keywordInfo = createKeywordInfo( entries );

        RifEclipseOutputFileTools::createResultEntries( keywordInfo,
                                                        { firstTimeStepInfo },
                                                        RiaDefines::ResultCatType::STATIC_NATIVE,
                                                        eclipseCaseData,
                                                        1 );
    }

    // Unit system
    {
        // Default units type is METRIC, look in restart file, then init file and then grid file until we find something
        RiaDefines::EclipseUnitSystem unitsType      = RiaDefines::EclipseUnitSystem::UNITS_METRIC;
        int                           unitsTypeValue = -1;

        if ( m_restartFile != nullptr )
        {
            if ( m_restartFile->hasArray( "INTEHEAD", 0 ) )
            {
                const auto& intHeader = m_restartFile->getRestartData<int>( "INTEHEAD", 0 );

                if ( intHeader.size() > 2 ) unitsTypeValue = intHeader[2];
            }
        }

        if ( unitsTypeValue < 0 )
        {
            if ( m_initFile != nullptr )
            {
                const auto& intHeader = m_initFile->getInitData<int>( "INTEHEAD" );
                if ( intHeader.size() > 2 ) unitsTypeValue = intHeader[2];
            }
        }

        if ( unitsTypeValue < 0 )
        {
            unitsTypeValue = m_gridUnit;
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

    auto task = progress.task( "Handling well information", 10 );
    if ( loadWellDataEnabled() && !m_restartFileName.empty() )
    {
        auto restartAccess = std::make_unique<RifEclipseUnifiedRestartFileAccess>();
        restartAccess->setRestartFiles( QStringList( QString::fromStdString( m_restartFileName ) ) );
        restartAccess->open();

        std::vector<QDateTime> filteredTimeSteps;
        for ( auto& a : filteredTimeStepInfos )
        {
            filteredTimeSteps.push_back( a.m_date );
        }

        RifReaderEclipseWell::readWellCells( restartAccess.get(),
                                             eclipseCaseData,
                                             filteredTimeSteps,
                                             m_gridNames,
                                             isImportOfCompleteMswDataEnabled() );

        restartAccess->close();
    }
    else
    {
        RiaLogging::info( "Skipping import of simulation well data" );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifReaderOpmCommon::TimeDataFile> RifReaderOpmCommon::readTimeSteps()
{
    setupInitAndRestartAccess();

    std::vector<RifReaderOpmCommon::TimeDataFile> reportTimeData;

    if ( m_restartFile == nullptr ) return reportTimeData;

    try
    {
        namespace VI = Opm::RestartIO::Helpers::VectorItems;

        for ( auto seqNum : m_restartFile->listOfReportStepNumbers() )
        {
            const std::string inteheadString = "INTEHEAD";
            const std::string doubheadString = "DOUBHEAD";

            if ( m_restartFile->hasArray( inteheadString, seqNum ) )
            {
                const auto& intehead = m_restartFile->getRestartData<int>( inteheadString, seqNum );
                auto        year     = intehead[VI::intehead::YEAR];
                auto        month    = intehead[VI::intehead::MONTH];
                auto        day      = intehead[VI::intehead::DAY];

                double daySinceSimStart = 0.0;

                if ( m_restartFile->hasArray( doubheadString, seqNum ) )
                {
                    const auto& doubhead = m_restartFile->getRestartData<double>( doubheadString, seqNum );
                    if ( !doubhead.empty() )
                    {
                        // Read out the simulation time from start from DOUBHEAD. There is no enum defined to access this value.
                        // https://github.com/OPM/ResInsight/issues/11092

                        daySinceSimStart = doubhead[0];
                    }
                }

                reportTimeData.emplace_back(
                    TimeDataFile{ .sequenceNumber = seqNum, .year = year, .month = month, .day = day, .simulationTimeFromStart = daySinceSimStart } );
            }
        }
    }
    catch ( std::exception& e )
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    return reportTimeData;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifReaderOpmCommon::timeStepsOnFile( QString gridFileName )
{
    locateInitAndRestartFiles( gridFileName );
    setupInitAndRestartAccess();

    if ( m_restartFile == nullptr ) return {};

    auto timeStepsFromFile = readTimeSteps();

    std::vector<QDateTime> dateTimes;

    for ( const auto& timeStep : timeStepsFromFile )
    {
        QDate     date( timeStep.year, timeStep.month, timeStep.day );
        QDateTime dateTime = RiaQDateTimeTools::createDateTime( date, Qt::UTC );
        dateTimes.push_back( dateTime );
    }

    return dateTimes;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::verifyActiveCellInfo( int activeSizeMat, int activeSizeFrac )
{
    if ( m_initFile == nullptr ) return true;

    int activeCells = 0;

    for ( const auto& name : m_gridNames )
    {
        activeCells += m_initFile->activeCells( name );
    }

    return activeCells == ( activeSizeFrac + activeSizeMat );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<std::vector<int>> RifReaderOpmCommon::readActiveCellInfoFromPorv( RigEclipseCaseData* eclipseCaseData, bool isDualPorosity )
{
    if ( m_initFile == nullptr ) return {};

    std::vector<std::vector<int>> activeCellsAllGrids;

    bool divideCellCountByTwo = isDualPorosity;

    const int nGrids = (int)m_gridNames.size();

    for ( int gridIdx = 0; gridIdx < nGrids; gridIdx++ )
    {
        auto porvValues = m_initFile->getInitData<float>( "PORV", m_gridNames[gridIdx] );

        int activeCellCount = (int)porvValues.size();
        if ( divideCellCountByTwo )
        {
            activeCellCount /= 2;
        }

        std::vector<int> activeCellsOneGrid;
        activeCellsOneGrid.resize( activeCellCount, 0 );

        for ( int poreValueIndex = 0; poreValueIndex < static_cast<int>( porvValues.size() ); poreValueIndex++ )
        {
            int indexToCell = poreValueIndex;
            if ( indexToCell >= activeCellCount )
            {
                indexToCell = poreValueIndex - activeCellCount;
            }

            if ( porvValues[poreValueIndex] > 0.0f )
            {
                if ( isDualPorosity )
                {
                    if ( poreValueIndex < activeCellCount )
                    {
                        activeCellsOneGrid[indexToCell] += (int)ActiveType::ACTIVE_MATRIX_VALUE;
                    }
                    else
                    {
                        activeCellsOneGrid[indexToCell] += (int)ActiveType::ACTIVE_FRACTURE_VALUE;
                    }
                }
                else
                {
                    activeCellsOneGrid[indexToCell] += (int)ActiveType::ACTIVE_MATRIX_VALUE;
                }
            }
        }

        activeCellsAllGrids.push_back( activeCellsOneGrid );
    }

    return activeCellsAllGrids;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::updateActiveCellInfo( RigEclipseCaseData*             eclipseCaseData,
                                               Opm::EclIO::EGrid&              opmGrid,
                                               std::vector<Opm::EclIO::EGrid>& lgrGrids,
                                               RigMainGrid*                    mainGrid )
{
    auto activeCellInfoPerGrid = readActiveCellInfoFromPorv( eclipseCaseData, opmGrid.porosity_mode() > 0 );

    int nInfos = (int)activeCellInfoPerGrid.size();
    if ( nInfos > 0 )
    {
        int gridIdx = 0;
        opmGrid.set_active_cells( activeCellInfoPerGrid[gridIdx++] );

        for ( auto& lgr : lgrGrids )
        {
            if ( gridIdx < nInfos ) lgr.set_active_cells( activeCellInfoPerGrid[gridIdx++] );
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
bool RifReaderOpmCommon::importActiveGrid( RigActiveCellGrid* activeGrid, RigEclipseCaseData* eclipseCaseData )
{
    caf::ProgressInfo progInfo( 5, "Importing Eclipse Grid" );

    Opm::EclIO::EGrid opmGrid( m_gridFileName );

    const auto& dims = opmGrid.dimension();
    activeGrid->setGridPointDimensions( cvf::Vec3st( dims[0] + 1, dims[1] + 1, dims[2] + 1 ) );
    activeGrid->setGridName( "Main grid" );
    activeGrid->setDualPorosity( opmGrid.porosity_mode() > 0 );

    // assign grid unit, if found (1 = Metric, 2 = Field, 3 = Lab)
    auto gridUnitStr = RiaStdStringTools::toUpper( opmGrid.grid_unit() );
    if ( gridUnitStr.starts_with( 'M' ) )
        m_gridUnit = 1;
    else if ( gridUnitStr.starts_with( 'F' ) )
        m_gridUnit = 2;
    else if ( gridUnitStr.starts_with( 'C' ) )
        m_gridUnit = 3;

    auto globalMatrixActiveSize   = opmGrid.activeCells();
    auto globalFractureActiveSize = opmGrid.activeFracCells();

    m_gridNames.clear();
    m_gridNames.push_back( "global" );

    std::vector<Opm::EclIO::EGrid> lgrGrids; // lgrs not supported here for now

    // active cell information
    {
        auto task = progInfo.task( "Getting Active Cell Information", 1 );

        // in case init file and grid file disagrees with number of active cells, read extra porv information from init file to correct this
        if ( !verifyActiveCellInfo( globalMatrixActiveSize, globalFractureActiveSize ) )
        {
            updateActiveCellInfo( eclipseCaseData, opmGrid, lgrGrids, activeGrid );
        }

        activeGrid->transferActiveInformation( eclipseCaseData,
                                               opmGrid.totalActiveCells(),
                                               opmGrid.activeCells(),
                                               opmGrid.activeFracCells(),
                                               opmGrid.active_indexes(),
                                               opmGrid.active_frac_indexes() );
    }

    // grid geometry
    {
        auto task = progInfo.task( "Loading Active Cell Main Grid Geometry", 1 );
        transferActiveGeometry( opmGrid, activeGrid, eclipseCaseData );
    }

    activeGrid->initAllSubGridsParentGridPointer();

    if ( isNNCsEnabled() )
    {
        auto task = progInfo.task( "Loading NNC data", 1 );
        transferStaticNNCData( opmGrid, lgrGrids, activeGrid );
    }

    auto opmMapAxes = opmGrid.get_mapaxes();
    if ( opmMapAxes.size() == 6 )
    {
        std::array<double, 6> mapAxes;
        for ( size_t i = 0; i < opmMapAxes.size(); ++i )
        {
            mapAxes[i] = opmMapAxes[i];
        }

        // Set the map axes transformation matrix on the main grid
        activeGrid->setMapAxes( mapAxes );
        activeGrid->setUseMapAxes( true );

        auto transform = activeGrid->mapAxisTransform();

        // Invert the transformation matrix to convert from file coordinates to domain coordinates
        transform.invert();

#pragma omp parallel for
        for ( long i = 0; i < static_cast<long>( activeGrid->nodes().size() ); i++ )
        {
            auto& n = activeGrid->nodes()[i];
            n.transformPoint( transform );
        }
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::transferActiveGeometry( Opm::EclIO::EGrid& opmMainGrid, RigActiveCellGrid* activeGrid, RigEclipseCaseData* eclipseCaseData )
{
    int cellCount = opmMainGrid.totalActiveCells();

    RigCell defaultCell;
    defaultCell.setHostGrid( activeGrid );
    for ( size_t i = 0; i < 8; i++ )
        defaultCell.cornerIndices()[i] = 0;

    activeGrid->globalCellArray().resize( cellCount + 1, defaultCell );
    activeGrid->globalCellArray()[cellCount].setInvalid( true );

    activeGrid->nodes().resize( ( cellCount + 1 ) * 8, cvf::Vec3d( 0, 0, 0 ) );

    auto& riNodes = activeGrid->nodes();

    opmMainGrid.loadData();
    opmMainGrid.load_grid_data();

    const bool  isRadialGrid      = opmMainGrid.is_radial();
    const auto& activeMatIndexes  = opmMainGrid.active_indexes();
    const auto& activeFracIndexes = opmMainGrid.active_frac_indexes();

    // Compute the center of the LGR radial grid cells for each K layer
    auto radialGridCenterTopLayerOpm = isRadialGrid
                                           ? RifOpmRadialGridTools::computeXyCenterForTopOfCells( opmMainGrid, opmMainGrid, activeGrid )
                                           : std::map<int, std::pair<double, double>>();

    // use same mapping as resdata
    const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( opmMainGrid.totalNumberOfCells() ); opmCellIndex++ )
    {
        if ( ( activeMatIndexes[opmCellIndex] < 0 ) && ( activeFracIndexes[opmCellIndex] < 0 ) ) continue;

        auto opmIJK = opmMainGrid.ijk_from_global_index( opmCellIndex );

        double xCenterCoordOpm = 0.0;
        double yCenterCoordOpm = 0.0;

        if ( isRadialGrid && radialGridCenterTopLayerOpm.contains( opmIJK[2] ) )
        {
            const auto& [xCenter, yCenter] = radialGridCenterTopLayerOpm[opmIJK[2]];
            xCenterCoordOpm                = xCenter;
            yCenterCoordOpm                = yCenter;
        }

        auto     riReservoirIndex = activeGrid->cellIndexFromIJK( opmIJK[0], opmIJK[1], opmIJK[2] );
        RigCell& cell             = activeGrid->globalCellArray()[riReservoirIndex];
        cell.setGridLocalCellIndex( riReservoirIndex );
        cell.setParentCellIndex( cvf::UNDEFINED_SIZE_T );

        // corner coordinates
        std::array<double, 8> opmX{};
        std::array<double, 8> opmY{};
        std::array<double, 8> opmZ{};
        opmMainGrid.getCellCorners( opmCellIndex, opmX, opmY, opmZ );

        // Each cell has 8 nodes, use reservoir cell index and multiply to find first node index for cell
        auto riNodeStartIndex = riReservoirIndex * 8;

        for ( size_t opmNodeIndex = 0; opmNodeIndex < 8; opmNodeIndex++ )
        {
            auto   riCornerIndex = cellMappingECLRi[opmNodeIndex];
            size_t riNodeIndex   = riNodeStartIndex + riCornerIndex;

            auto& riNode = riNodes[riNodeIndex];
            riNode.x()   = opmX[opmNodeIndex] + xCenterCoordOpm;
            riNode.y()   = opmY[opmNodeIndex] + yCenterCoordOpm;
            riNode.z()   = -opmZ[opmNodeIndex];

            cell.cornerIndices()[riCornerIndex] = riNodeIndex;
        }

        cell.setInvalid( cell.isLongPyramidCell() );
    }
}
