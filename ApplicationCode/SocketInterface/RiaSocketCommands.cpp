//
//  Copyright (C) 2011-2012 Statoil ASA, Ceetron AS
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

#include "RiaStdInclude.h"
#include "RiaSocketServer.h"


#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimReservoirCellResultsCacher.h"

#include "RimCase.h"
#include "RigCaseData.h"
#include "RigCaseCellResultsData.h"


class RiaGetTimeStepDates : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetTimeStepDates"); }
    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;

        if (args.size() == 2)
        {
            argCaseGroupId = args[1].toInt();
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);

        bool canFetchData = true;

        if (!rimCase || !rimCase->reservoirData())
        {
            canFetchData = false;
        }

        size_t scalarIndexWithMaxTimeStepCount = cvf::UNDEFINED_SIZE_T;
        if (rimCase && rimCase->reservoirData())
        {
            rimCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->maxTimeStepCount(&scalarIndexWithMaxTimeStepCount);
            if (scalarIndexWithMaxTimeStepCount == cvf::UNDEFINED_SIZE_T)
            {
                canFetchData = false;
            }
        }

        // Did not find any result to fetch data from, return zero data found
        if (!canFetchData)
        {
            quint64 timeStepCount = 0;
            quint64 byteCount = sizeof(quint64);

            socketStream << byteCount;
            socketStream << timeStepCount;

            return true;
        }

        std::vector<QDateTime> timeStepDates = rimCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->timeStepDates(scalarIndexWithMaxTimeStepCount);

        quint64 timeStepCount = timeStepDates.size();
        quint64 byteCount = sizeof(quint64) + 6 * timeStepCount * sizeof(qint32);

        socketStream << byteCount;
        socketStream << timeStepCount;

        for (size_t i = 0; i < timeStepCount; i++)
        {
            qint32 intValue = 0;
            
            intValue = timeStepDates[i].date().year();
            socketStream << intValue;

            intValue = timeStepDates[i].date().month();
            socketStream << intValue;

            intValue = timeStepDates[i].date().day();
            socketStream << intValue;

            intValue = timeStepDates[i].time().hour();
            socketStream << intValue;

            intValue = timeStepDates[i].time().minute();
            socketStream << intValue;

            intValue = timeStepDates[i].time().second();
            socketStream << intValue;
        }

        return true;
    }

};

static bool RiaGetTimeStepDates_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetTimeStepDates>(RiaGetTimeStepDates::commandName());



class RiaGetTimeStepDays : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetTimeStepDays"); }
    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;

        if (args.size() == 2)
        {
            argCaseGroupId = args[1].toInt();
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);

        bool canFetchData = true;

        if (!rimCase || !rimCase->reservoirData())
        {
            canFetchData = false;
        }

        size_t scalarIndexWithMaxTimeStepCount = cvf::UNDEFINED_SIZE_T;
        if (rimCase && rimCase->reservoirData())
        {
            rimCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->maxTimeStepCount(&scalarIndexWithMaxTimeStepCount);
            if (scalarIndexWithMaxTimeStepCount == cvf::UNDEFINED_SIZE_T)
            {
                canFetchData = false;
            }
        }

        // Did not find any result to fetch data from, return zero data found
        if (!canFetchData)
        {
            quint64 timeStepCount = 0;
            quint64 byteCount = sizeof(quint64);

            socketStream << byteCount;
            socketStream << timeStepCount;

            return true;
        }

        std::vector<QDateTime> timeStepDates = rimCase->reservoirData()->results(RifReaderInterface::MATRIX_RESULTS)->timeStepDates(scalarIndexWithMaxTimeStepCount);

        quint64 timeStepCount = timeStepDates.size();
        quint64 byteCount = sizeof(quint64) + timeStepCount * sizeof(qint32);

        socketStream << byteCount;
        socketStream << timeStepCount;

        if (timeStepCount > 0)
        {
            double secondsInADay = 24 * 60 * 60;

            for (size_t i = 0; i < timeStepCount; i++)
            {
                double secondsDiff = timeStepDates[0].secsTo(timeStepDates[i]);

                double decimalDaysDiff = secondsDiff / secondsInADay;

                socketStream << decimalDaysDiff;
            }
        }

        return true;
    }

};

static bool RiaGetTimeStepDays_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetTimeStepDays>(RiaGetTimeStepDays::commandName());




class RiaGetGridDimensions : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetGridDimensions"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;

        if (args.size() == 2)
        {
            argCaseGroupId = args[1].toInt();
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->reservoirData() || !rimCase->reservoirData()->mainGrid())
        {
            quint64 byteCount = 0;

            socketStream << byteCount;

            return true;
        }

        // Write data back to octave: I, J, K dimensions


        if (rimCase && rimCase->reservoirData() && rimCase->reservoirData()->mainGrid())
        {
            std::vector<RigGridBase*> grids;
            rimCase->reservoirData()->allGrids(&grids);

            quint64 byteCount = grids.size() * 3 * sizeof(quint64);
            socketStream << byteCount;

            for (size_t i = 0; i < grids.size(); i++)
            {
                size_t iCount = 0;
                size_t jCount = 0;
                size_t kCount = 0;

                iCount = grids[i]->cellCountI();
                jCount = grids[i]->cellCountJ();
                kCount = grids[i]->cellCountK();

                socketStream << (quint64)iCount << (quint64)jCount << (quint64)kCount;
            }
        }

        return true;
    }
};

static bool RiaGetGridDimensions_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetGridDimensions>(RiaGetGridDimensions::commandName());
