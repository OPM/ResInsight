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
#include "RifOpmGridTools.h"

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
bool RifReaderOpmCommon::open( const QString& fileName, RigEclipseCaseData* eclipseCase )
{
    caf::ProgressInfo progress( 100, "Reading Grid" );

    QStringList fileSet;
    if ( !RifEclipseOutputFileTools::findSiblingFilesWithSameBaseName( fileName, &fileSet ) ) return false;

    try
    {
        m_gridFileName = fileName.toStdString();

        if ( !RifOpmGridTools::importGrid( m_gridFileName, eclipseCase->mainGrid(), eclipseCase ) )
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

                RigMainGrid* mainGrid = eclipseCase->mainGrid();
                mainGrid->setFaults( faults );
            }
        }

        {
            auto task = progress.task( "Reading Results Meta data", 50 );
            buildMetaData( eclipseCase );
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
bool RifReaderOpmCommon::staticResult( const QString& result, RiaDefines::PorosityModelType matrixOrFracture, std::vector<double>* values )
{
    if ( m_initFile )
    {
        try
        {
            auto resultName = result.toStdString();

            auto resultEntries = m_initFile->getList();
            for ( const auto& entry : resultEntries )
            {
                const auto& [keyword, kwType, size] = entry;
                if ( keyword == resultName )
                {
                    if ( kwType == EclIO::eclArrType::REAL )
                    {
                        auto fileValues = m_initFile->getInitData<float>( resultName );
                        values->insert( values->end(), fileValues.begin(), fileValues.end() );
                    }
                    else if ( kwType == EclIO::eclArrType::DOUB )
                    {
                        auto fileValues = m_initFile->getInitData<double>( resultName );
                        values->insert( values->end(), fileValues.begin(), fileValues.end() );
                    }
                    else if ( kwType == EclIO::eclArrType::INTE )
                    {
                        auto fileValues = m_initFile->getInitData<int>( resultName );
                        values->insert( values->end(), fileValues.begin(), fileValues.end() );
                    }
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

            auto stepNumbers = m_restartFile->listOfReportStepNumbers();
            auto stepNumber  = stepNumbers[stepIndex];

            auto resultEntries = m_restartFile->getList();
            for ( const auto& entry : resultEntries )
            {
                const auto& [keyword, kwType, size] = entry;
                if ( keyword == resultName )
                {
                    if ( kwType == EclIO::eclArrType::DOUB )
                    {
                        auto fileValues = m_restartFile->getRestartData<double>( resultName, stepNumber );
                        values->insert( values->end(), fileValues.begin(), fileValues.end() );
                    }
                    if ( kwType == EclIO::eclArrType::REAL )
                    {
                        auto fileValues = m_restartFile->getRestartData<float>( resultName, stepNumber );
                        values->insert( values->end(), fileValues.begin(), fileValues.end() );
                    }
                    else if ( kwType == EclIO::eclArrType::INTE )
                    {
                        auto fileValues = m_restartFile->getRestartData<int>( resultName, stepNumber );
                        values->insert( values->end(), fileValues.begin(), fileValues.end() );
                    }
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
std::vector<RifKeywordValueCount> createKeywordInfo( std::vector<EclIO::EclFile::EclEntry> entries )
{
    std::vector<RifKeywordValueCount> fileKeywordInfo;

    for ( const auto& entry : entries )
    {
        const auto& [keyword, kwType, size] = entry;

        RifKeywordValueCount::KeywordDataType dataType = RifKeywordValueCount::KeywordDataType::UNKNOWN;

        if ( kwType == EclIO::eclArrType::INTE )
            dataType = RifKeywordValueCount::KeywordDataType::INTEGER;
        else if ( kwType == EclIO::eclArrType::REAL )
            dataType = RifKeywordValueCount::KeywordDataType::FLOAT;
        else if ( kwType == EclIO::eclArrType::DOUB )
            dataType = RifKeywordValueCount::KeywordDataType::DOUBLE;

        if ( dataType != RifKeywordValueCount::KeywordDataType::UNKNOWN )
        {
            fileKeywordInfo.emplace_back( RifKeywordValueCount( keyword, static_cast<size_t>( size ), dataType ) );
        }
    }

    return fileKeywordInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::buildMetaData( RigEclipseCaseData* eclipseCase )
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
            auto reportEntries = m_restartFile->listOfRstArrays( reportNumber );
            entries.insert( entries.end(), reportEntries.begin(), reportEntries.end() );
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

        std::vector<RifKeywordValueCount> keywordInfo = createKeywordInfo( entries );

        RifEclipseOutputFileTools::createResultEntries( keywordInfo, timeStepInfos, RiaDefines::ResultCatType::DYNAMIC_NATIVE, eclipseCase );

        firstTimeStepInfo = timeStepInfos.front();

        readWellCells( m_restartFile, eclipseCase, m_timeSteps );
    }

    if ( !initFileName.empty() )
    {
        m_initFile = std::make_unique<EclIO::EInit>( initFileName );

        auto                              entries     = m_initFile->list_arrays();
        std::vector<RifKeywordValueCount> keywordInfo = createKeywordInfo( entries );

        RifEclipseOutputFileTools::createResultEntries( keywordInfo, { firstTimeStepInfo }, RiaDefines::ResultCatType::STATIC_NATIVE, eclipseCase );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RifReaderOpmCommon::readWellCells( std::shared_ptr<EclIO::ERst>  restartFile,
                                        RigEclipseCaseData*           eclipseCase,
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
                auto cellIndex = eclipseCase->mainGrid()->cellIndexFromIJK( rstWell.ij[0], rstWell.ij[1], 0 );
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
                        auto cellIndex = eclipseCase->mainGrid()->cellIndexFromIJK( conn.ijk[0], conn.ijk[1], conn.ijk[2] );
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

    eclipseCase->setSimWellData( wells );
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
                auto intehead = restartFile->getRestartData<int>( inteheadString, seqNum );
                auto year     = intehead[VI::intehead::YEAR];
                auto month    = intehead[VI::intehead::MONTH];
                auto day      = intehead[VI::intehead::DAY];

                double daySinceSimStart = 0.0;

                if ( restartFile->hasArray( doubheadString, seqNum ) )
                {
                    auto doubhead = restartFile->getRestartData<double>( doubheadString, seqNum );
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
