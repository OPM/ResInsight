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

#include "RiaSocketTools.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaSocketDataTransfer.h"
#include "RiaSocketServer.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCaseCollection.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimIdenticalGridCaseGroup.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimSimWellInViewCollection.h"

#include "cvfTimer.h"

#include <QErrorMessage>
#include <QTcpSocket>

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RiaSocketTools::findCaseFromArgs(RiaSocketServer* server, const QList<QByteArray>& args)
{
    RimEclipseCase* rimCase = nullptr;
    int caseId = -1;

    if (args.size() > 1)
    {
        caseId = args[1].toInt();
    }
    rimCase = server->findReservoir(caseId);

    if (rimCase == nullptr)
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

    RimEclipseCase* eclCase = dynamic_cast<RimEclipseCase*> (rimCase);
    RimCaseCollection* caseCollection = nullptr;
    if (eclCase)
    {
        caseCollection = eclCase->parentCaseCollection();
    }

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

        if (dynamic_cast<RimEclipseInputCase*>(rimCase))
        {
            caseType = "InputCase";
        }
        else if (eclCase)
        {
            caseType = "ResultCase";
        }
        else
        {
            caseType = "GeoMechCase";
        }
    }
}

//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
bool RiaSocketTools::writeBlockData(RiaSocketServer* server, QTcpSocket* socket, const char* data, quint64 bytesToWrite)
{
    cvf::Timer timer;

    QStringList errorMessages;
    bool writeSucceded = RiaSocketDataTransfer::writeBlockDataToSocket(socket, data, bytesToWrite, errorMessages);

    if (server)
    {
        for (int i = 0; i < errorMessages.size(); i++)
        {
            server->errorMessageDialog()->showMessage(errorMessages[i]);
        }

//         double totalTimeMS = timer.time() * 1000.0;
//         QString resultInfo = QString("Total time '%1 ms'").arg(totalTimeMS);
// 
//         server->errorMessageDialog()->showMessage(resultInfo);
    }

    return writeSucceded;
}
