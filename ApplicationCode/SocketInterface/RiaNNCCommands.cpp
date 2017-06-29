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
        if (!rimCase) return true;

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
        socketStream << (qint32)i << (qint32)j << (qint32)k;
    }
};

static bool RiaGetNNCConnections_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetNNCConnections>(RiaGetNNCConnections::commandName());
