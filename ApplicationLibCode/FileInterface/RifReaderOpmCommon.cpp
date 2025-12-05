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
#include "RiaOpmParserTools.h"
#include "RiaPreferencesSystem.h"
#include "RiaQDateTimeTools.h"
#include "RiaResultNames.h"
#include "RiaStdStringTools.h"

#include "RifEclipseOutputFileTools.h"
#include "RifEclipseReportKeywords.h"
#include "RifEclipseUnifiedRestartFileAccess.h"
#include "RifEdfmTools.h"
#include "RifOpmRadialGridTools.h"
#include "RifReaderEclipseWell.h"

#include "RigActiveCellGrid.h"
#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"
#include "Well/RigSimWellData.h"
#include "Well/RigWellResultFrame.h"

#include "cafProgressInfo.h"

#include "opm/io/eclipse/EGrid.hpp"
#include "opm/io/eclipse/EInit.hpp"
#include "opm/io/eclipse/ERst.hpp"
#include "opm/output/eclipse/VectorItems/intehead.hpp"

#include <QStringList>

#include <iostream>

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

        if ( !importGrid( eclipseCaseData->mainGrid(), eclipseCaseData ) )
        {
            RiaLogging::error( "Failed to open grid file " + fileName );

            return false;
        }

        auto iLimitFromEdfm = RifEdfmTools::checkForEdfmLimitI( fileName );
        if ( iLimitFromEdfm > 0 )
        {
            eclipseCaseData->mainGrid()->invalidateCellsAboveI( iLimitFromEdfm - 1 /* zero based index */ );
        }

        if ( isFaultImportEnabled() )
        {
            auto task = progress.task( "Reading faults", 25 );

            cvf::Collection<RigFault> faults;

            importFaults( fileSet, &faults );

            RigMainGrid* mainGrid = eclipseCaseData->mainGrid();
            mainGrid->setFaults( faults );
        }

        m_eclipseCaseData = eclipseCaseData;

        {
            auto task = progress.task( "Reading Results Meta data", 25 );
            buildMetaData( eclipseCaseData, progress );
        }

        if ( m_radialGridDetected )
        {
            if ( readerSettings().useCylindricalCoordinates )
            {
                auto task         = progress.task( "Check for Radial Grid", 25 );
                bool isLgrCreated = RifOpmRadialGridTools::importCylindricalCoordinates( fileName.toStdString(), m_eclipseCaseData );
                if ( isLgrCreated )
                {
                    m_eclipseCaseData->clearWellCellsInGridCache();
                }
            }
            else
            {
                RifOpmRadialGridTools::tryConvertRadialGridToCartesianGrid( fileName.toStdString(), eclipseCaseData->mainGrid() );
            }
        }

        if ( isNNCsEnabled() )
        {
            auto task = progress.task( "Handling NCC Result data", 25 );

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
        RiaLogging::debug( e.what() );

        QString errorMsg = "Unable to read cell data from grid.\nOpen Preferences->Grid->EGRID settings, and set 'Model Reader' to "
                           "'resdata' and try again.";
        RiaLogging::error( errorMsg );

        return false;
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

    m_radialGridDetected = opmGrid.is_radial();

    const auto& dims = opmGrid.dimension();
    mainGrid->setCellCounts( cvf::Vec3st( dims[0], dims[1], dims[2] ) );
    mainGrid->setGridName( "Main grid" );
    mainGrid->setDualPorosity( opmGrid.porosity_mode() > 0 );

    // assign grid unit, if found (1 = Metric, 2 = Field, 3 = Lab)
    // Note: Accepts "m" or "M" as abbreviation for "METRES" (METRIC units)
    auto gridUnitStr = RiaStdStringTools::toUpper( opmGrid.grid_unit() );
    if ( gridUnitStr.starts_with( 'M' ) )
    {
        m_gridUnit = 1;
        RiaLogging::debug(
            QString( "Grid unit from EGRID file: '%1' (interpreted as METRIC)" ).arg( QString::fromStdString( opmGrid.grid_unit() ) ) );
    }
    else if ( gridUnitStr.starts_with( 'F' ) )
    {
        m_gridUnit = 2;
        RiaLogging::debug(
            QString( "Grid unit from EGRID file: '%1' (interpreted as FIELD)" ).arg( QString::fromStdString( opmGrid.grid_unit() ) ) );
    }
    else if ( gridUnitStr.starts_with( 'C' ) )
    {
        m_gridUnit = 3;
        RiaLogging::debug(
            QString( "Grid unit from EGRID file: '%1' (interpreted as LAB)" ).arg( QString::fromStdString( opmGrid.grid_unit() ) ) );
    }
    else if ( !gridUnitStr.empty() )
    {
        RiaLogging::warning( QString( "Unknown grid unit from EGRID file: '%1'" ).arg( QString::fromStdString( opmGrid.grid_unit() ) ) );
    }

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
        localGrid->setCellCounts( cvf::Vec3st( lgrDims[0], lgrDims[1], lgrDims[2] ) );
        localGrid->setGridId( lgrIdx + 1 );
        localGrid->setGridName( lgr_names[lgrIdx] );
        localGrid->setIndexToStartOfCells( totalCellCount );
        mainGrid->addLocalGrid( localGrid );

        totalCellCount += lgrGrids[lgrIdx].totalNumberOfCells();
    }

    // active cell information
    {
        RigActiveCellInfo* activeCellInfo         = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
        RigActiveCellInfo* fractureActiveCellInfo = eclipseCaseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

        activeCellInfo->setReservoirCellCount( totalCellCount );
        fractureActiveCellInfo->setReservoirCellCount( totalCellCount );
        activeCellInfo->setGridCount( 1 + numLGRs );
        fractureActiveCellInfo->setGridCount( 1 + numLGRs );

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

    // grid geometry
    {
        auto task = progInfo.task( "Loading Main Grid Geometry", 1 );
        transferGeometry( opmGrid, opmGrid, mainGrid, mainGrid, eclipseCaseData );

        bool hasParentInfo = ( lgr_parent_names.size() >= (size_t)numLGRs );

        auto task2 = progInfo.task( "Loading LGR Grid Geometry ", 1 );

        for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
        {
            RigGridBase* parentGrid = hasParentInfo ? mainGrid->gridByName( lgr_parent_names[lgrIdx] ) : mainGrid;

            RigLocalGrid* localGrid = static_cast<RigLocalGrid*>( mainGrid->gridById( lgrIdx + 1 ) );
            localGrid->setParentGrid( parentGrid );

            transferGeometry( opmGrid, lgrGrids[lgrIdx], mainGrid, localGrid, eclipseCaseData );
        }
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

        double norm_denominator = mapAxes[2] * mapAxes[5] - mapAxes[4] * mapAxes[3];

        // Set the map axes transformation matrix on the main grid
        mainGrid->setMapAxes( mapAxes );
        mainGrid->setUseMapAxes( norm_denominator != 0.0 );

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
    size_t cellStartIndex = mainGrid->reservoirCells().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    const bool invalidateLongPyramidCells = invalidateLongThinCells();

    RigCell defaultCell;
    defaultCell.setHostGrid( localGrid );

    mainGrid->reservoirCells().resize( cellStartIndex + cellCount, defaultCell );

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
        RigCell& cell             = mainGrid->cell( cellStartIndex + riReservoirIndex );
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
        if ( invalidateLongPyramidCells )
        {
            cell.setInvalid( cell.isLongPyramidCell() );
        }
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
                        // Do not try to read data that is not present in the file
                        if ( !m_restartFile->hasArray( resultName, stepNumber ) ) continue;

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
        try
        {
            m_initFile = std::make_unique<EclIO::EInit>( m_initFileName );
        }
        catch ( ... )
        {
            m_initFile = nullptr;
        }
    }

    if ( ( m_restartFile == nullptr ) && !m_restartFileName.empty() )
    {
        try
        {
            auto startTime = RiaLogging::currentTime();

            m_restartFile = std::make_unique<EclIO::ERst>( m_restartFileName );

            const bool isLoggingEnabled = RiaPreferencesSystem::current()->isLoggingActivatedForKeyword( "RifReaderOpmCommon" );
            if ( isLoggingEnabled )
                RiaLogging::logElapsedTime( "Import of meta data from " + QString::fromStdString( m_restartFileName ), startTime );
        }
        catch ( ... )
        {
            m_restartFile = nullptr;
        }
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

    if ( timeStepsOnFile.empty() ) return timeStepInfos;

    auto  startDayOffset = timeStepsOnFile[0].simulationTimeFromStart;
    QDate startDate( timeStepsOnFile[0].year, timeStepsOnFile[0].month, timeStepsOnFile[0].day );

    for ( size_t i = 0; i < timeStepsOnFile.size(); i++ )
    {
        if ( isTimeStepIncludedByFilter( i ) )
        {
            auto dateTime = dateTimeFromTimeStepOnFile( timeStepsOnFile[i], startDate, startDayOffset );

            timeStepInfos.push_back(
                RigEclipseTimeStepInfo( dateTime, timeStepsOnFile[i].sequenceNumber, timeStepsOnFile[i].simulationTimeFromStart ) );
        }
    }

    return timeStepInfos;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
QDateTime RifReaderOpmCommon::dateTimeFromTimeStepOnFile( RifReaderOpmCommon::TimeDataFile timeOnFile, QDate startDate, double startDayOffset )
{
    QDateTime dateTime;
    if ( timeOnFile.simulationTimeFromStart == 0 )
    {
        QDate date( timeOnFile.year, timeOnFile.month, timeOnFile.day );
        dateTime = RiaQDateTimeTools::createDateTime( date, Qt::TimeSpec::UTC );
    }
    else
    {
        dateTime = RiaQDateTimeTools::createDateTime( startDate, Qt::TimeSpec::UTC );

        double    dayDoubleValue   = timeOnFile.simulationTimeFromStart;
        int       dayValue         = cvf::Math::floor( dayDoubleValue );
        const int adjustedDayValue = dayValue - startDayOffset;
        dateTime                   = dateTime.addDays( adjustedDayValue );

        double dayFraction  = dayDoubleValue - dayValue;
        double milliseconds = dayFraction * 24.0 * 60.0 * 60.0 * 1000.0;

        dateTime = dateTime.addMSecs( milliseconds );
    }

    return dateTime;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::buildMetaData( RigEclipseCaseData* eclipseCaseData, caf::ProgressInfo& progress )
{
    setupInitAndRestartAccess();

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
        std::optional<RiaDefines::EclipseUnitSystem> egridUnit = RifEclipseOutputFileTools::unitValueToEnum( m_gridUnit );
        std::optional<RiaDefines::EclipseUnitSystem> initUnit;
        std::optional<RiaDefines::EclipseUnitSystem> unrstUnit;

        namespace VI = Opm::RestartIO::Helpers::VectorItems;

        // Read from init file
        if ( m_initFile != nullptr )
        {
            const auto& intHeader = m_initFile->getInitData<int>( "INTEHEAD" );
            if ( intHeader.size() > 2 )
            {
                initUnit = RifEclipseOutputFileTools::unitValueToEnum( intHeader[2] );
            }
        }

        // Read from restart file
        if ( m_restartFile != nullptr )
        {
            for ( auto reportStep : m_restartFile->listOfReportStepNumbers() )
            {
                if ( m_restartFile->hasArray( "INTEHEAD", reportStep ) )
                {
                    const auto& intHeader = m_restartFile->getRestartData<int>( "INTEHEAD", reportStep );
                    if ( intHeader.size() > VI::intehead::UNIT )
                    {
                        unrstUnit = RifEclipseOutputFileTools::unitValueToEnum( intHeader[VI::intehead::UNIT] );
                        break;
                    }
                }
            }
        }

        // Determine final unit system using hierarchy (egrid > init > unrst) with warnings
        RiaDefines::EclipseUnitSystem unitsType = RifEclipseOutputFileTools::determineUnitSystem( egridUnit, initUnit, unrstUnit );
        m_eclipseCaseData->setUnitsType( unitsType );
    }

    // Set available phases from INTEHEAD
    {
        auto phases = availablePhases();
        m_eclipseCaseData->setAvailablePhases( phases );
    }

    auto task = progress.task( "Handling well information", 10 );
    if ( loadWellDataEnabled() && !m_restartFileName.empty() )
    {
        auto startTime = RiaLogging::currentTime();

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

        RiaLogging::logElapsedTime( "Import of simulation well data", startTime );
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
std::vector<int> RifReaderOpmCommon::readInteheadKeyword() const
{
    if ( m_initFile )
    {
        try
        {
            return m_initFile->getInitData<int>( "INTEHEAD" );
        }
        catch ( const std::exception& e )
        {
            RiaLogging::warning( QString( "Failed to read INTEHEAD keyword from init file: %1" ).arg( e.what() ) );
        }
    }

    return {};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
int RifReaderOpmCommon::readPhasesFromIntehead() const
{
    namespace VI = Opm::RestartIO::Helpers::VectorItems;

    auto intehead = readInteheadKeyword();
    if ( intehead.size() > VI::intehead::PHASE )
    {
        return intehead[VI::intehead::PHASE];
    }

    return -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::set<RiaDefines::PhaseType> RifReaderOpmCommon::availablePhases() const
{
    int phaseIndicator = readPhasesFromIntehead();
    return RiaOpmParserTools::phasesFromInteheadValue( phaseIndicator );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<QDateTime> RifReaderOpmCommon::timeStepsOnFile( QString gridFileName )
{
    locateInitAndRestartFiles( gridFileName );
    setupInitAndRestartAccess();

    if ( m_restartFile == nullptr ) return {};

    auto timeStepsOnFile = readTimeSteps();
    if ( timeStepsOnFile.empty() ) return {};

    auto  startDayOffset = timeStepsOnFile[0].simulationTimeFromStart;
    QDate startDate( timeStepsOnFile[0].year, timeStepsOnFile[0].month, timeStepsOnFile[0].day );

    std::vector<QDateTime> dateTimes;

    for ( const auto& timeStep : timeStepsOnFile )
    {
        auto dateTime = dateTimeFromTimeStepOnFile( timeStep, startDate, startDayOffset );
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

    bool hasThermalData = m_initFile->hasArray( RiaResultNames::rockv().toStdString() );

    bool divideCellCountByTwo = isDualPorosity;

    const int nGrids = (int)m_gridNames.size();

    for ( int gridIdx = 0; gridIdx < nGrids; gridIdx++ )
    {
        auto porvValues = m_initFile->getInitData<float>( RiaResultNames::porv().toStdString(), m_gridNames[gridIdx] );
        auto rockValues = hasThermalData ? m_initFile->getInitData<float>( RiaResultNames::rockv().toStdString(), m_gridNames[gridIdx] )
                                         : std::vector<float>();

        rockValues.resize( porvValues.size(), 0.0f ); // Ensure rockValues has same size as porvValues, fill with 0.0f if not present

        int activeCellCount = (int)porvValues.size();
        if ( divideCellCountByTwo )
        {
            activeCellCount /= 2;
        }

        std::vector<int> activeCellsOneGrid;
        activeCellsOneGrid.resize( activeCellCount, 0 );

        for ( int resultIndex = 0; resultIndex < static_cast<int>( porvValues.size() ); resultIndex++ )
        {
            int indexToCell = resultIndex;
            if ( indexToCell >= activeCellCount )
            {
                indexToCell = resultIndex - activeCellCount;
            }

            if ( porvValues[resultIndex] > 0.0f )
            {
                if ( isDualPorosity )
                {
                    if ( resultIndex < activeCellCount )
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
            if ( rockValues[resultIndex] > 0.0f )
            {
                if ( activeCellsOneGrid[indexToCell] == 0 )
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
