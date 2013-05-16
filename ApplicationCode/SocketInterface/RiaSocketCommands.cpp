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
        quint64 byteCount = sizeof(quint64) + timeStepCount * sizeof(qint32);

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
