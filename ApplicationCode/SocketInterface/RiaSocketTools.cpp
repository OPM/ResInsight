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

#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RiaSocketTools.h"
#include "RiaSocketServer.h"
#include "RimCase.h"
#include "RimCaseCollection.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimInputCase.h"

#include "RimReservoirView.h"
#include "RimResultSlot.h"
#include "RimCellEdgeResultSlot.h"
#include "RimCellRangeFilterCollection.h"
#include "RimCellPropertyFilterCollection.h"
#include "RimWellCollection.h"
#include "Rim3dOverlayInfoConfig.h"
#include "RimReservoirCellResultsCacher.h"

#include "RimInputPropertyCollection.h"

#include <QTcpSocket>


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimCase* RiaSocketTools::findCaseFromArgs(RiaSocketServer* server, const QList<QByteArray>& args)
{
    RimCase* rimCase = NULL;
    int caseId = -1;

    if (args.size() > 1)
    {
        caseId = args[1].toInt();
    }
    rimCase = server->findReservoir(caseId);

    if (rimCase == NULL)
    {
        // TODO: Display error message a different place to avoid socket comm to be halted.
        //server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the Case with CaseId : \"%1\"").arg(caseId));
    }

    return rimCase;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaSocketTools::getCaseInfoFromCase(RimCase* rimCase, qint64& caseId, QString& caseName, QString& caseType, qint64& caseGroupId)
{
    CVF_ASSERT(rimCase);

    caseId = rimCase->caseId;
    caseName = rimCase->caseUserDescription;

    RimCaseCollection* caseCollection = rimCase->parentCaseCollection();
    if (caseCollection)
    {
        caseGroupId = caseCollection->parentCaseGroup()->groupId;

        if (RimIdenticalGridCaseGroup::isStatisticsCaseCollection(caseCollection))
        {
            caseType = "StatisticsCase";
        }
        else
        {
            caseType = "SourceCase";
        }
    }
    else
    {
        caseGroupId = -1;

        if (dynamic_cast<RimInputCase*>(rimCase))
        {
            caseType = "InputCase";
        }
        else
        {
            caseType = "ResultCase";
        }
    }
}

bool RiaSocketTools::writeBlockData(RiaSocketServer* server, QTcpSocket* socket, const char* data, quint64 bytesToWrite)
{
    cvf::Timer timer;

    quint64 bytesWritten = 0;
    int blockCount = 0;

    quint64 maxBlockSize = RiaApplication::instance()->preferences()->blockSize();


    while (bytesWritten < bytesToWrite)
    {
        quint64 byteCountToWrite = qMin(bytesToWrite - bytesWritten, maxBlockSize);

        qint64 actuallyBytesWritten = socket->write(data + bytesWritten, byteCountToWrite);
        if (actuallyBytesWritten < 0)
        {
            if (server)
            {
                QString errorMessage = "Error detected when writing data, error string from socket : \n" + socket->errorString();

                server->errorMessageDialog()->showMessage(errorMessage);
            }

            return false;
        }

        bytesWritten += actuallyBytesWritten;

        blockCount++;
    }

    if (server)
    {
        double totalTimeMS = timer.time() * 1000.0;
        QString resultInfo = QString("Total time '%1 ms'\nTotal bytes written .  %2\nNumber of blocks : %3\nBlock size : %4").arg(totalTimeMS).arg(bytesWritten).arg(blockCount).arg(maxBlockSize);

        server->errorMessageDialog()->showMessage(resultInfo);
    }

    return true;
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSocketTools::readBlockData(QTcpSocket& socket, char* data, quint64 bytesToRead, QStringList& errorMessages)
{
    quint64 bytesRead = 0;
    int blockCount = 0;

    quint64 maxBlockSize = RiaApplication::instance()->preferences()->blockSize();

    while (bytesRead < bytesToRead)
    {
        if (socket.bytesAvailable())
        {
            quint64 byteCountToRead = qMin(bytesToRead - bytesRead, maxBlockSize);

            qint64 actuallyBytesRead = socket.read(data + bytesRead, byteCountToRead);
            if (actuallyBytesRead < 0)
            {
                errorMessages.push_back("Error detected when writing data, error string from socket");
                errorMessages.push_back(socket.errorString());

                return false;
            }

            bytesRead += actuallyBytesRead;
            blockCount++;
        }
        else
        {
            if (!socket.waitForReadyRead())
            {
                errorMessages.push_back("Waited for data for %1 milli seconds.");
                errorMessages.push_back(socket.errorString());

                return false;
            }
        }
    }

    return true;
}

