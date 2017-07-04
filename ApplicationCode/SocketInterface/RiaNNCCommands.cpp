/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017 Statoil ASA
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
#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimReservoirCellResultsStorage.h"

#include <QTcpSocket>
#include <QErrorMessage>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetNNCConnections: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetNNCConnections"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        // Write data back to octave: columnCount, GridNr I J K GridNr I J K
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            // No data available
            socketStream << (quint64)0;
            return true;
        }

        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();

        size_t connectionCount = mainGrid->nncData()->connections().size();

        socketStream << (quint64)connectionCount;

        for (const RigConnection& connection : mainGrid->nncData()->connections())
        {
            const RigCell& cell1 = mainGrid->globalCellArray()[connection.m_c1GlobIdx];
            const RigCell& cell2 = mainGrid->globalCellArray()[connection.m_c2GlobIdx];

            sendCellInfo(socketStream, cell1);
            sendCellInfo(socketStream, cell2);
        }

        return true;
    }

    static void sendCellInfo(QDataStream& socketStream, const RigCell& cell)
    {
        RigGridBase* hostGrid = cell.hostGrid();
        size_t gridLocalCellIndex = cell.gridLocalCellIndex();
        size_t i, j, k;
        hostGrid->ijkFromCellIndex(gridLocalCellIndex, &i, &j, &k);

        socketStream << (qint32)hostGrid->gridIndex();
        socketStream << (qint32)(i+1) << (qint32)(j+1) << (qint32)(k+1);
    }
};

static bool RiaGetNNCConnections_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetNNCConnections>(RiaGetNNCConnections::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetDynamicNNCValues: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetDynamicNNCValues"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        // Write data back to octave: connectionCount, timeStepCount, property values
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0;
            return true;
        }

        QString propertyName = args[2];

        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();
        const std::vector< std::vector<double> >* nncValues = mainGrid->nncData()->dynamicConnectionScalarResultByName(propertyName);

        if (nncValues == nullptr)
        {
            socketStream << (quint64)0 << (quint64)0;
            return true;
        }

        std::vector<size_t> requestedTimeSteps;
        if (args.size() > 3)
        {
            bool timeStepReadError = false;
            for (int argIdx = 3; argIdx < args.size(); ++argIdx)
            {
                bool conversionOk = false;
                int tsIdx = args[argIdx].toInt(&conversionOk);

                if (conversionOk)
                {
                    requestedTimeSteps.push_back(tsIdx);
                }
                else
                {
                    timeStepReadError = true;
                }
            }

            if (timeStepReadError)
            {
                server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riGetDynamicNNCValues : \n") + RiaSocketServer::tr("An error occurred while interpreting the requested time steps."));
            }
        }
        else
        {
            for (size_t timeStep = 0; timeStep < nncValues->size(); ++timeStep)
            {
                requestedTimeSteps.push_back(timeStep);
            }
        }

        // then the connection count and time step count.
        size_t connectionCount = mainGrid->nncData()->connections().size();
        size_t timeStepCount = requestedTimeSteps.size();

        socketStream << (quint64)connectionCount;
        socketStream << (quint64)timeStepCount;

        for (size_t timeStep : requestedTimeSteps)
        {
            const std::vector<double>& timeStepValues = nncValues->at(timeStep);
            RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)timeStepValues.data(), sizeof(double) * timeStepValues.size());
        }

        return true;
    }
};

static bool RiaGetDynamicNNCValues_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetDynamicNNCValues>(RiaGetDynamicNNCValues::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetStaticNNCValues: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetStaticNNCValues"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        QString propertyName = args[2];

        // Write data back to octave: connectionCount, property values
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()))
        {
            // No data available
            socketStream << (quint64)0;
            return true;
        }

        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();
        const std::vector<double>* nncValues = mainGrid->nncData()->staticConnectionScalarResultByName(propertyName);

        if (nncValues == nullptr)
        {
            socketStream << (quint64)0;
            return true;
        }

        // connection count
        size_t connectionCount = mainGrid->nncData()->connections().size();
        socketStream << (quint64)connectionCount;

        RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)nncValues->data(), sizeof(double) * nncValues->size());

        return true;
    }
};

static bool RiaGetStaticNNCValues_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetStaticNNCValues>(RiaGetStaticNNCValues::commandName());
