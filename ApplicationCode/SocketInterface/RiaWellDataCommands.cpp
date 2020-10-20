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

#include "RiaSocketCommand.h"
#include "RiaSocketServer.h"
#include "RiaSocketTools.h"

#include "RigEclipseCaseData.h"
#include "RigGridBase.h"
#include "RigSimWellData.h"

#include "RimEclipseCase.h"

#include "cvfCollection.h"

#include <QTcpSocket>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetWellNames : public RiaSocketCommand
{
public:
    static QString commandName() { return QString( "GetWellNames" ); }

    bool interpretCommand( RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream ) override
    {
        int             caseId  = args[1].toInt();
        RimEclipseCase* rimCase = server->findReservoir( caseId );
        if ( !rimCase )
        {
            server->showErrorMessage( RiaSocketServer::tr( "ResInsight SocketServer: \n" ) +
                                      RiaSocketServer::tr( "Could not find the case with ID : \"%1\"" ).arg( caseId ) );

            return true;
        }

        std::vector<QString> wellNames;

        const cvf::Collection<RigSimWellData>& wells = rimCase->eclipseCaseData()->wellResults();

        for ( size_t wIdx = 0; wIdx < wells.size(); ++wIdx )
        {
            wellNames.push_back( wells[wIdx]->m_wellName );
        }

        quint64 byteCount = sizeof( quint64 );
        quint64 wellCount = wellNames.size();

        for ( size_t wIdx = 0; wIdx < wellCount; wIdx++ )
        {
            byteCount += wellNames[wIdx].size() * sizeof( QChar );
        }

        socketStream << byteCount;
        socketStream << wellCount;

        for ( size_t wIdx = 0; wIdx < wellCount; wIdx++ )
        {
            socketStream << wellNames[wIdx];
        }

        return true;
    }
};

static bool RiaGetWellNames_init =
    RiaSocketCommandFactory::instance()->registerCreator<RiaGetWellNames>( RiaGetWellNames::commandName() );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetWellStatus : public RiaSocketCommand
{
public:
    static QString commandName() { return QString( "GetWellStatus" ); }

    bool interpretCommand( RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream ) override
    {
        int     caseId   = args[1].toInt();
        QString wellName = args[2];

        RimEclipseCase* rimCase = server->findReservoir( caseId );
        if ( !rimCase )
        {
            server->showErrorMessage( RiaSocketServer::tr( "ResInsight SocketServer: \n" ) +
                                      RiaSocketServer::tr( "Could not find the case with ID : \"%1\"" ).arg( caseId ) );

            return true;
        }

        // Create a list of all the requested time steps

        std::vector<size_t> requestedTimesteps;
        // First find the well result for the correct well

        const cvf::Collection<RigSimWellData>& allWellRes = rimCase->eclipseCaseData()->wellResults();
        cvf::ref<RigSimWellData>               currentWellResult;
        for ( size_t tsIdx = 0; tsIdx < allWellRes.size(); ++tsIdx )
        {
            if ( allWellRes[tsIdx]->m_wellName == wellName )
            {
                currentWellResult = allWellRes[tsIdx];
                break;
            }
        }

        if ( currentWellResult.isNull() )
        {
            server->showErrorMessage( RiaSocketServer::tr( "ResInsight SocketServer: \n" ) +
                                      RiaSocketServer::tr( "Could not find the well with name : \"%1\"" ).arg( wellName ) );

            return true;
        }

        if ( args.size() <= 3 )
        {
            // Select all timesteps.

            for ( size_t tsIdx = 0; tsIdx < currentWellResult->m_resultTimeStepIndexToWellTimeStepIndex.size(); ++tsIdx )
            {
                requestedTimesteps.push_back( tsIdx );
            }
        }
        else
        {
            bool timeStepReadError = false;
            for ( int argIdx = 3; argIdx < args.size(); ++argIdx )
            {
                bool conversionOk = false;
                int  tsIdx        = args[argIdx].toInt( &conversionOk );

                if ( conversionOk )
                {
                    requestedTimesteps.push_back( tsIdx );
                }
                else
                {
                    timeStepReadError = true;
                }
            }

            if ( timeStepReadError )
            {
                server->showErrorMessage(
                    RiaSocketServer::tr( "ResInsight SocketServer: riGetGridProperty : \n" ) +
                    RiaSocketServer::tr( "An error occurred while interpreting the requested timesteps." ) );
            }
        }

        std::vector<QString> wellTypes;
        std::vector<qint32>  wellStatuses;

        for ( size_t tsIdx = 0; tsIdx < requestedTimesteps.size(); ++tsIdx )
        {
            QString wellType   = "NotDefined";
            qint32  wellStatus = 0;
            if ( currentWellResult->hasWellResult( tsIdx ) )
            {
                switch ( currentWellResult->wellResultFrame( tsIdx ).m_productionType )
                {
                    case RigWellResultFrame::PRODUCER:
                        wellType = "Producer";
                        break;
                    case RigWellResultFrame::OIL_INJECTOR:
                        wellType = "OilInjector";
                        break;
                    case RigWellResultFrame::WATER_INJECTOR:
                        wellType = "WaterInjector";
                        break;
                    case RigWellResultFrame::GAS_INJECTOR:
                        wellType = "GasInjector";
                        break;
                }

                wellStatus = currentWellResult->wellResultFrame( tsIdx ).m_isOpen ? 1 : 0;
            }

            wellTypes.push_back( wellType );
            wellStatuses.push_back( wellStatus );
        }

        quint64 byteCount     = sizeof( quint64 );
        quint64 timeStepCount = wellTypes.size();

        for ( size_t tsIdx = 0; tsIdx < timeStepCount; tsIdx++ )
        {
            byteCount += wellTypes[tsIdx].size() * sizeof( QChar );
            byteCount += sizeof( qint32 );
        }

        socketStream << byteCount;
        socketStream << timeStepCount;

        for ( size_t tsIdx = 0; tsIdx < timeStepCount; tsIdx++ )
        {
            socketStream << wellTypes[tsIdx];
            socketStream << wellStatuses[tsIdx];
        }

        return true;
    }
};

static bool RiaGetWellStatus_init =
    RiaSocketCommandFactory::instance()->registerCreator<RiaGetWellStatus>( RiaGetWellStatus::commandName() );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetWellCells : public RiaSocketCommand
{
public:
    static QString commandName() { return QString( "GetWellCells" ); }

    bool interpretCommand( RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream ) override
    {
        int     caseId      = args[1].toInt();
        QString wellName    = args[2];
        size_t  timeStepIdx = args[3].toInt() - 1; // Interpret timeStepIdx from octave as 1-based

        RimEclipseCase* rimCase = server->findReservoir( caseId );
        if ( !rimCase )
        {
            server->showErrorMessage( RiaSocketServer::tr( "ResInsight SocketServer: \n" ) +
                                      RiaSocketServer::tr( "Could not find the case with ID : \"%1\"" ).arg( caseId ) );

            socketStream << (quint64)0;
            return true;
        }

        const cvf::Collection<RigSimWellData>& allWellRes = rimCase->eclipseCaseData()->wellResults();
        cvf::ref<RigSimWellData>               currentWellResult;
        for ( size_t cIdx = 0; cIdx < allWellRes.size(); ++cIdx )
        {
            if ( allWellRes[cIdx]->m_wellName == wellName )
            {
                currentWellResult = allWellRes[cIdx];
                break;
            }
        }

        if ( currentWellResult.isNull() )
        {
            server->showErrorMessage( RiaSocketServer::tr( "ResInsight SocketServer: \n" ) +
                                      RiaSocketServer::tr( "Could not find the well with name : \"%1\"" ).arg( wellName ) );

            socketStream << (quint64)0;
            return true;
        }

        if ( !currentWellResult->hasWellResult( timeStepIdx ) )
        {
            socketStream << (quint64)0;
            return true;
        }

        std::vector<qint32> cellIs;
        std::vector<qint32> cellJs;
        std::vector<qint32> cellKs;
        std::vector<qint32> gridIndices;
        std::vector<qint32> cellStatuses;
        std::vector<qint32> branchIds;
        std::vector<qint32> segmentIds;

        // Fetch results
        const RigWellResultFrame& wellResFrame = currentWellResult->wellResultFrame( timeStepIdx );
        std::vector<RigGridBase*> grids;
        rimCase->eclipseCaseData()->allGrids( &grids );

        for ( size_t bIdx = 0; bIdx < wellResFrame.m_wellResultBranches.size(); ++bIdx )
        {
            const std::vector<RigWellResultPoint>& branchResPoints =
                wellResFrame.m_wellResultBranches[bIdx].m_branchResultPoints;
            for ( size_t rpIdx = 0; rpIdx < branchResPoints.size(); ++rpIdx )
            {
                const RigWellResultPoint& resPoint = branchResPoints[rpIdx];

                if ( resPoint.isCell() )
                {
                    size_t i;
                    size_t j;
                    size_t k;
                    size_t gridIdx = resPoint.m_gridIndex;
                    grids[gridIdx]->ijkFromCellIndex( resPoint.m_gridCellIndex, &i, &j, &k );
                    bool isOpen    = resPoint.m_isOpen;
                    int  branchId  = resPoint.m_ertBranchId;
                    int  segmentId = resPoint.m_ertSegmentId;

                    cellIs.push_back( static_cast<qint32>( i + 1 ) ); // NB: 1-based index in Octave
                    cellJs.push_back( static_cast<qint32>( j + 1 ) ); // NB: 1-based index in Octave
                    cellKs.push_back( static_cast<qint32>( k + 1 ) ); // NB: 1-based index in Octave
                    gridIndices.push_back( static_cast<qint32>( gridIdx ) );
                    cellStatuses.push_back( static_cast<qint32>( isOpen ) );
                    branchIds.push_back( branchId );
                    segmentIds.push_back( segmentId );
                }
            }
        }

        quint64 byteCount = sizeof( quint64 );
        quint64 cellCount = cellIs.size();

        byteCount += cellCount * ( 7 * sizeof( qint32 ) );

        socketStream << byteCount;
        socketStream << cellCount;

        for ( size_t cIdx = 0; cIdx < cellCount; cIdx++ )
        {
            socketStream << cellIs[cIdx];
            socketStream << cellJs[cIdx];
            socketStream << cellKs[cIdx];
            socketStream << gridIndices[cIdx];
            socketStream << cellStatuses[cIdx];
            socketStream << branchIds[cIdx];
            socketStream << segmentIds[cIdx];
        }

        return true;
    }
};

static bool RiaGetWellCells_init =
    RiaSocketCommandFactory::instance()->registerCreator<RiaGetWellCells>( RiaGetWellCells::commandName() );
