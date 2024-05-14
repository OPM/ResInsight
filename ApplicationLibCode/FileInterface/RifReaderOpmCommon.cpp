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

#include "RifEclipseOutputFileTools.h"
#include "RifEclipseReportKeywords.h"
#include "RifEclipseRestartDataAccess.h"
#include "RifReaderEclipseWell.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigSimWellData.h"
#include "RigWellResultFrame.h"

#include "cafProgressInfo.h"

#include "opm/input/eclipse/Deck/Deck.hpp"
#include "opm/input/eclipse/EclipseState/Runspec.hpp"
#include "opm/input/eclipse/Parser/Parser.hpp"
#include "opm/input/eclipse/Schedule/Well/Connection.hpp"
#include "opm/input/eclipse/Schedule/Well/Well.hpp"
#include "opm/io/eclipse/EGrid.hpp"
#include "opm/io/eclipse/EInit.hpp"
#include "opm/io/eclipse/ERst.hpp"
#include "opm/io/eclipse/RestartFileView.hpp"
#include "opm/io/eclipse/rst/state.hpp"
#include "opm/output/eclipse/VectorItems/group.hpp"
#include "opm/output/eclipse/VectorItems/intehead.hpp"
#include "opm/output/eclipse/VectorItems/well.hpp"

#include <iostream>

using namespace Opm;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RifReaderOpmCommon::RifReaderOpmCommon()
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
bool RifReaderOpmCommon::open( const QString& fileName, RigEclipseCaseData* caseData )
{
    caf::ProgressInfo progress( 100, "Reading Grid" );

    QStringList fileSet;
    if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &fileSet ) ) return false;

    try
    {
        m_gridFileName = fileName.toStdString();

        if ( !importGrid( caseData->mainGrid(), caseData ) )
        {
            RiaLogging::error( "Failed to open grid file " + fileName );

            return false;
        }

        {
            auto task = progress.task( "Reading faults", 25 );

            if ( isFaultImportEnabled() )
            {
                cvf::Collection<RigFault> faults;

                importFaults( fileSet, &faults );

                RigMainGrid* mainGrid = caseData->mainGrid();
                mainGrid->setFaults( faults );
            }
        }

        {
            auto task = progress.task( "Reading Results Meta data", 25 );
            buildMetaData( caseData, progress );
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
bool RifReaderOpmCommon::importGrid( RigMainGrid* mainGrid, RigEclipseCaseData* caseData )
{
    Opm::EclIO::EGrid opmGrid( m_gridFileName );

    RigActiveCellInfo* activeCellInfo         = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    const auto& dims = opmGrid.dimension();
    mainGrid->setGridPointDimensions( cvf::Vec3st( dims[0] + 1, dims[1] + 1, dims[2] + 1 ) );
    mainGrid->setGridName( "Main grid" );
    mainGrid->setDualPorosity( opmGrid.porosity_mode() > 0 );

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

    caf::ProgressInfo progInfo( 3 + numLGRs, "" );

    {
        auto task = progInfo.task( "Loading Main Grid Data", 3 );
        transferGeometry( opmGrid, opmGrid, mainGrid, mainGrid, caseData, 0, 0 );
    }

    activeCellInfo->setGridCount( 1 + numLGRs );
    fractureActiveCellInfo->setGridCount( 1 + numLGRs );

    activeCellInfo->setGridActiveCellCounts( 0, globalMatrixActiveSize );
    fractureActiveCellInfo->setGridActiveCellCounts( 0, globalFractureActiveSize );

    bool hasParentInfo = ( lgr_parent_names.size() >= (size_t)numLGRs );

    for ( int lgrIdx = 0; lgrIdx < numLGRs; lgrIdx++ )
    {
        auto task = progInfo.task( "LGR number " + QString::number( lgrIdx + 1 ), 1 );

        RigGridBase* parentGrid = hasParentInfo ? mainGrid->gridByName( lgr_parent_names[lgrIdx] ) : mainGrid;

        RigLocalGrid* localGrid = static_cast<RigLocalGrid*>( mainGrid->gridById( lgrIdx + 1 ) );
        localGrid->setParentGrid( parentGrid );

        transferGeometry( opmGrid, lgrGrids[lgrIdx], mainGrid, localGrid, caseData, globalMatrixActiveSize, globalFractureActiveSize );

        int matrixActiveCellCount = lgrGrids[lgrIdx].activeCells();
        globalMatrixActiveSize += matrixActiveCellCount;
        activeCellInfo->setGridActiveCellCounts( lgrIdx + 1, matrixActiveCellCount );

        int fractureActiveCellCount = lgrGrids[lgrIdx].activeFracCells();
        globalFractureActiveSize += fractureActiveCellCount;
        fractureActiveCellInfo->setGridActiveCellCounts( lgrIdx + 1, fractureActiveCellCount );
    }

    mainGrid->initAllSubGridsParentGridPointer();
    activeCellInfo->computeDerivedData();
    fractureActiveCellInfo->computeDerivedData();

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
void RifReaderOpmCommon::transferGeometry( Opm::EclIO::EGrid&  opmMainGrid,
                                           Opm::EclIO::EGrid&  opmGrid,
                                           RigMainGrid*        mainGrid,
                                           RigGridBase*        localGrid,
                                           RigEclipseCaseData* caseData,
                                           size_t              matrixActiveStartIndex,
                                           size_t              fractureActiveStartIndex )
{
    int    cellCount      = opmGrid.totalNumberOfCells();
    size_t cellStartIndex = mainGrid->globalCellArray().size();
    size_t nodeStartIndex = mainGrid->nodes().size();

    RigActiveCellInfo* activeCellInfo         = caseData->activeCellInfo( RiaDefines::PorosityModelType::MATRIX_MODEL );
    RigActiveCellInfo* fractureActiveCellInfo = caseData->activeCellInfo( RiaDefines::PorosityModelType::FRACTURE_MODEL );

    RigCell defaultCell;
    defaultCell.setHostGrid( localGrid );
    mainGrid->globalCellArray().resize( cellStartIndex + cellCount, defaultCell );

    mainGrid->nodes().resize( nodeStartIndex + cellCount * 8, cvf::Vec3d( 0, 0, 0 ) );

    auto& riNodes = mainGrid->nodes();

    opmGrid.loadData();
    opmGrid.load_grid_data();

    // same mapping as libecl
    const size_t cellMappingECLRi[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

    const auto host_parentcell = opmGrid.hostCellsGlobalIndex();

#pragma omp parallel for
    for ( int opmCellIndex = 0; opmCellIndex < static_cast<int>( localGrid->cellCount() ); opmCellIndex++ )
    {
        auto opmIJK = opmGrid.ijk_from_global_index( opmCellIndex );

        auto     riReservoirIndex = localGrid->cellIndexFromIJK( opmIJK[0], opmIJK[1], opmIJK[2] );
        RigCell& cell             = mainGrid->globalCellArray()[cellStartIndex + riReservoirIndex];
        cell.setGridLocalCellIndex( riReservoirIndex );

        // active cell index
        int matrixActiveIndex = opmGrid.active_index( opmIJK[0], opmIJK[1], opmIJK[2] );
        if ( matrixActiveIndex != -1 )
        {
            activeCellInfo->setCellResultIndex( cellStartIndex + opmCellIndex, matrixActiveStartIndex + matrixActiveIndex );
        }

        int fractureActiveIndex = opmGrid.active_frac_index( opmIJK[0], opmIJK[1], opmIJK[2] );
        if ( fractureActiveIndex != -1 )
        {
            fractureActiveCellInfo->setCellResultIndex( cellStartIndex + opmCellIndex, fractureActiveStartIndex + fractureActiveIndex );
        }

        // parent cell index
        if ( ( host_parentcell.size() > (size_t)opmCellIndex ) && host_parentcell[opmCellIndex] >= 0 )
        {
            cell.setParentCellIndex( host_parentcell[opmCellIndex] );
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
            riNode.x()   = opmX[opmNodeIndex];
            riNode.y()   = opmY[opmNodeIndex];
            riNode.z()   = -opmZ[opmNodeIndex];

            cell.cornerIndices()[riCornerIndex] = riNodeIndex;
        }

        cell.setInvalid( cell.isLongPyramidCell() );
    }

    // subgrid pointers
    RigLocalGrid* realLocalGrid = dynamic_cast<RigLocalGrid*>( localGrid );
    RigGridBase*  parentGrid    = realLocalGrid != nullptr ? realLocalGrid->parentGrid() : nullptr;

    if ( parentGrid != nullptr )
    {
        for ( auto localCellInGlobalIdx : opmGrid.hostCellsGlobalIndex() )
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
                            values->insert( values->end(), fileValues.begin(), fileValues.end() );
                        }
                        else if ( kwType == EclIO::eclArrType::DOUB )
                        {
                            const auto& fileValues = m_initFile->getInitData<double>( resultName, gridName );
                            values->insert( values->end(), fileValues.begin(), fileValues.end() );
                        }
                        else if ( kwType == EclIO::eclArrType::INTE )
                        {
                            const auto& fileValues = m_initFile->getInitData<int>( resultName, gridName );
                            values->insert( values->end(), fileValues.begin(), fileValues.end() );
                        }
                    }
                    break;
                }
            }

            // Always clear data after reading to avoid memory use
            m_initFile->clearData();

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

            const auto& stepNumbers = m_restartFile->listOfReportStepNumbers();
            auto        stepNumber  = stepNumbers[stepIndex];

            auto resultEntries = m_restartFile->getList();
            for ( const auto& entry : resultEntries )
            {
                const auto& [keyword, kwType, size] = entry;
                if ( keyword == resultName )
                {
                    for ( auto& gridName : m_gridNames )
                    {
                        if ( gridName == "global" ) // main grid
                        {
                            if ( kwType == EclIO::eclArrType::DOUB )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<double>( resultName, stepNumber );
                                values->insert( values->end(), fileValues.begin(), fileValues.end() );
                            }
                            if ( kwType == EclIO::eclArrType::REAL )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<float>( resultName, stepNumber );
                                values->insert( values->end(), fileValues.begin(), fileValues.end() );
                            }
                            else if ( kwType == EclIO::eclArrType::INTE )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<int>( resultName, stepNumber );
                                values->insert( values->end(), fileValues.begin(), fileValues.end() );
                            }
                        }
                        else
                        {
                            if ( kwType == EclIO::eclArrType::DOUB )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<double>( resultName, stepNumber, gridName );
                                values->insert( values->end(), fileValues.begin(), fileValues.end() );
                            }
                            if ( kwType == EclIO::eclArrType::REAL )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<float>( resultName, stepNumber, gridName );
                                values->insert( values->end(), fileValues.begin(), fileValues.end() );
                            }
                            else if ( kwType == EclIO::eclArrType::INTE )
                            {
                                const auto& fileValues = m_restartFile->getRestartData<int>( resultName, stepNumber, gridName );
                                values->insert( values->end(), fileValues.begin(), fileValues.end() );
                            }
                        }
                    }
                    break;
                }
            }

            // Always clear data after reading to avoid memory use
            m_restartFile->clearData();

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
void RifReaderOpmCommon::buildMetaData( RigEclipseCaseData* eclipseCaseData, caf::ProgressInfo& progress )
{
    auto getFileNameForType = []( RiaEclipseFileNameTools::EclipseFileType fileType, const QString& candidate ) -> std::string
    {
        const QString ext = caf::AppEnum<RiaEclipseFileNameTools::EclipseFileType>::text( fileType );
        if ( candidate.endsWith( ext, Qt::CaseInsensitive ) ) return candidate.toStdString();
        return {};
    };

    std::string initFileName;
    std::string restartFileName;

    QStringList fileSet;
    RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( QString::fromStdString( m_gridFileName ), &fileSet );
    for ( const auto& s : fileSet )
    {
        auto initCandidate    = getFileNameForType( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_INIT, s );
        auto restartCandidate = getFileNameForType( RiaEclipseFileNameTools::EclipseFileType::ECLIPSE_UNRST, s );

        if ( !initCandidate.empty() ) initFileName = initCandidate;
        if ( !restartCandidate.empty() ) restartFileName = restartCandidate;
    }

    RigEclipseTimeStepInfo firstTimeStepInfo{ QDateTime(), 0, 0.0 };
    if ( !restartFileName.empty() )
    {
        m_restartFile = std::make_shared<EclIO::ERst>( restartFileName );

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

        auto timeStepsFromFile = readTimeSteps( m_restartFile );

        std::vector<RigEclipseTimeStepInfo> timeStepInfos;
        std::vector<QDateTime>              dateTimes;
        for ( const auto& timeStep : timeStepsFromFile )
        {
            QDate     date( timeStep.year, timeStep.month, timeStep.day );
            QDateTime dateTime = RiaQDateTimeTools::createDateTime( date );
            dateTimes.push_back( dateTime );
            timeStepInfos.emplace_back( dateTime, timeStep.sequenceNumber, timeStep.simulationTimeFromStart );
        }
        m_timeSteps = dateTimes;

        auto last = std::unique( entries.begin(), entries.end() );
        entries.erase( last, entries.end() );

        std::vector<RifEclipseKeywordValueCount> keywordInfo = createKeywordInfo( entries );

        RifEclipseOutputFileTools::createResultEntries( keywordInfo, timeStepInfos, RiaDefines::ResultCatType::DYNAMIC_NATIVE, eclipseCaseData );

        firstTimeStepInfo = timeStepInfos.front();
    }

    if ( !initFileName.empty() )
    {
        m_initFile = std::make_unique<EclIO::EInit>( initFileName );

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
                                                        eclipseCaseData );
    }

    auto task = progress.task( "Handling well information", 10 );
    if ( !isSkipWellData() && !restartFileName.empty() )
    {
        auto restartAccess = RifEclipseOutputFileTools::createDynamicResultAccess( QString::fromStdString( m_gridFileName ) );
        restartAccess->open();

        RifReaderEclipseWell::readWellCells( restartAccess.p(), eclipseCaseData, m_timeSteps, m_gridNames, isImportOfCompleteMswDataEnabled() );

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
void RifReaderOpmCommon::readWellCells( std::shared_ptr<EclIO::ERst>  restartFile,
                                        RigEclipseCaseData*           eclipseCaseData,
                                        const std::vector<QDateTime>& timeSteps )
{
    // It is required to create a deck as the input parameter to runspec. The default() initialization of the runspec keyword does not
    // initialize the object as expected.

    Deck    deck;
    Runspec runspec( deck );
    Parser  parser( false );

    cvf::Collection<RigSimWellData> wells;

    try
    {
        if ( restartFile->numberOfReportSteps() != timeSteps.size() )
        {
            RiaLogging::error( "Number of time steps is not matching number of report steps" );
            return;
        }

        auto getWellStateAndNames = [&]() -> std::pair<std::vector<RestartIO::RstState>, std::set<std::string>>
        {
            std::vector<RestartIO::RstState> states;
            std::set<std::string>            wellNames;

            for ( auto seqNumber : restartFile->listOfReportStepNumbers() )
            {
                auto fileView = std::make_shared<EclIO::RestartFileView>( restartFile, seqNumber );

                auto state = RestartIO::RstState::load( fileView, runspec, parser );
                states.emplace_back( state );

                for ( const auto& w : state.wells )
                {
                    wellNames.insert( w.name );
                }
            }

            return std::make_pair( states, wellNames );
        };

        auto [states, wellNames] = getWellStateAndNames();

        for ( const auto& wellName : wellNames )
        {
            cvf::ref<RigSimWellData> simWellData = new RigSimWellData;
            simWellData->setWellName( QString::fromStdString( wellName ) );
            simWellData->m_wellCellsTimeSteps.resize( timeSteps.size() );

            for ( size_t timeIdx = 0; timeIdx < timeSteps.size(); ++timeIdx )
            {
                auto state = states[timeIdx];

                auto it = std::find_if( state.wells.begin(),
                                        state.wells.end(),
                                        [&wellName]( const RestartIO::RstWell& well ) { return well.name == wellName; } );
                if ( it == state.wells.end() ) continue;

                RestartIO::RstWell rstWell = *it;

                RigWellResultFrame& wellResFrame = simWellData->m_wellCellsTimeSteps[timeIdx];
                wellResFrame.setTimestamp( timeSteps[timeIdx] );

                auto wellType = [&rstWell]() -> RiaDefines::WellProductionType
                {
                    if ( rstWell.wtype.producer() ) return RiaDefines::WellProductionType::PRODUCER;

                    if ( rstWell.wtype.injector_type() == InjectorType::WATER ) return RiaDefines::WellProductionType::WATER_INJECTOR;
                    if ( rstWell.wtype.injector_type() == InjectorType::GAS ) return RiaDefines::WellProductionType::GAS_INJECTOR;
                    if ( rstWell.wtype.injector_type() == InjectorType::OIL ) return RiaDefines::WellProductionType::OIL_INJECTOR;

                    return RiaDefines::WellProductionType::UNDEFINED_PRODUCTION_TYPE;
                };

                wellResFrame.setProductionType( wellType() );

                wellResFrame.setIsOpen( rstWell.well_status == RestartIO::Helpers::VectorItems::IWell::Value::Status::Open );

                // Well head
                RigWellResultPoint wellHead;
                wellHead.setGridIndex( 0 );
                auto cellIndex = eclipseCaseData->mainGrid()->cellIndexFromIJK( rstWell.ij[0], rstWell.ij[1], 0 );
                wellHead.setGridCellIndex( cellIndex );

                wellResFrame.setWellHead( wellHead );

                // Grid cells
                if ( !rstWell.connections.empty() )
                {
                    RigWellResultBranch wellResultBranch;
                    wellResultBranch.setErtBranchId( 0 ); // Normal wells have only one branch

                    std::vector<RigWellResultPoint> branchResultPoints = wellResultBranch.branchResultPoints();
                    const size_t                    existingCellCount  = branchResultPoints.size();
                    branchResultPoints.reserve( existingCellCount + rstWell.connections.size() );

                    for ( const auto& conn : rstWell.connections )
                    {
                        RigWellResultPoint wellResPoint;
                        wellResPoint.setGridIndex( 0 );
                        auto cellIndex = eclipseCaseData->mainGrid()->cellIndexFromIJK( conn.ijk[0], conn.ijk[1], conn.ijk[2] );
                        wellResPoint.setGridCellIndex( cellIndex );

                        wellResPoint.setIsOpen( conn.state == Connection::State::OPEN );

                        // All units for a Connection is given in SI units
                        // Convert back to the original units
                        // See RestartIO::RstConnection::RstConnection
                        auto us = state.unit_system;
                        wellResPoint.setFlowData( us.from_si( UnitSystem::measure::rate, conn.resv_rate ),
                                                  us.from_si( UnitSystem::measure::liquid_surface_rate, conn.oil_rate ),
                                                  us.from_si( UnitSystem::measure::gas_surface_rate, conn.gas_rate ),
                                                  us.from_si( UnitSystem::measure::liquid_surface_rate, conn.water_rate ) );
                        wellResPoint.setConnectionFactor( us.from_si( UnitSystem::measure::transmissibility, conn.cf ) );

                        branchResultPoints.push_back( wellResPoint );
                    }
                    wellResultBranch.setBranchResultPoints( branchResultPoints );
                    wellResFrame.addWellResultBranch( wellResultBranch );
                }
            }

            simWellData->computeMappingFromResultTimeIndicesToWellTimeIndices( timeSteps );
            wells.push_back( simWellData.p() );
        }
    }
    catch ( std::exception& e )
    {
        std::cout << "Exception: " << e.what() << std::endl;
    }

    eclipseCaseData->setSimWellData( wells );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RifReaderOpmCommon::TimeDataFile> RifReaderOpmCommon::readTimeSteps( std::shared_ptr<EclIO::ERst> restartFile )
{
    std::vector<RifReaderOpmCommon::TimeDataFile> reportTimeData;
    try
    {
        namespace VI = Opm::RestartIO::Helpers::VectorItems;

        for ( auto seqNum : restartFile->listOfReportStepNumbers() )
        {
            const std::string inteheadString = "INTEHEAD";
            const std::string doubheadString = "DOUBHEAD";

            if ( restartFile->hasArray( inteheadString, seqNum ) )
            {
                const auto& intehead = restartFile->getRestartData<int>( inteheadString, seqNum );
                auto        year     = intehead[VI::intehead::YEAR];
                auto        month    = intehead[VI::intehead::MONTH];
                auto        day      = intehead[VI::intehead::DAY];

                double daySinceSimStart = 0.0;

                if ( restartFile->hasArray( doubheadString, seqNum ) )
                {
                    const auto& doubhead = restartFile->getRestartData<double>( doubheadString, seqNum );
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
