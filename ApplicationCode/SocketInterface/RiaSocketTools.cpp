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


