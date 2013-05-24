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
#include "RiaSocketCommand.h"
#include "RiaSocketServer.h"
#include "RiaSocketTools.h"
 
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




//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetActiveCellProperty: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetActiveCellProperty"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {

        RimCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);

        QString propertyName = args[2];
        QString porosityModelName = args[3];

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName == "Fracture")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        // Find the requested data

        size_t scalarResultIndex = cvf::UNDEFINED_SIZE_T;
        std::vector< std::vector<double> >* scalarResultFrames = NULL;

        if (rimCase && rimCase->results(porosityModelEnum))
        {
            scalarResultIndex = rimCase->results(porosityModelEnum)->findOrLoadScalarResult(propertyName);

            if (scalarResultIndex != cvf::UNDEFINED_SIZE_T)
            {
                scalarResultFrames = &(rimCase->results(porosityModelEnum)->cellResults()->cellScalarResults(scalarResultIndex));
            }

        }

        if (scalarResultFrames == NULL)
        {
            server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: \n") + RiaSocketServer::tr("Could not find the %1 model property named: \"%2\"").arg(porosityModelName).arg(propertyName));
        }

        // Write data back : timeStepCount, bytesPrTimestep, dataForTimestep0 ... dataForTimestepN

        if ( scalarResultFrames == NULL)
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
        }
        else
        {
            // Create a list of all the requested timesteps

            std::vector<size_t> requestedTimesteps;

            if (args.size() <= 4)
            {
                // Select all
                for (size_t tsIdx = 0; tsIdx < scalarResultFrames->size(); ++tsIdx)
                {
                    requestedTimesteps.push_back(tsIdx);
                }
            }
            else
            {
                bool timeStepReadError = false;
                for (int argIdx = 4; argIdx < args.size(); ++argIdx)
                {
                    bool conversionOk = false;
                    int tsIdx = args[argIdx].toInt(&conversionOk);

                    if (conversionOk)
                    {
                        requestedTimesteps.push_back(tsIdx);
                    }
                    else
                    {
                        timeStepReadError = true;
                    }
                }

                if (timeStepReadError)
                {
                    server->errorMessageDialog()->showMessage(RiaSocketServer::tr("ResInsight SocketServer: riGetActiveCellProperty : \n") + RiaSocketServer::tr("An error occured while interpreting the requested timesteps."));
                }

            }

            // First write timestep count
            quint64 timestepCount = (quint64)requestedTimesteps.size();
            socketStream << timestepCount;

            // then the byte-size of the result values in one timestep

            const RigActiveCellInfo* activeInfo = rimCase->reservoirData()->activeCellInfo(porosityModelEnum);
            size_t  timestepResultCount = activeInfo->globalActiveCellCount();

            quint64 timestepByteCount = (quint64)(timestepResultCount*sizeof(double));
            socketStream << timestepByteCount ;

            // Then write the data.

            size_t globalCellCount = activeInfo->globalCellCount();
            for (size_t tIdx = 0; tIdx < requestedTimesteps.size(); ++tIdx)
            {
                for (size_t gcIdx = 0; gcIdx < globalCellCount; ++gcIdx)
                {
                    size_t resultIdx = activeInfo->cellResultIndex(gcIdx);
                    if (resultIdx != cvf::UNDEFINED_SIZE_T)
                    {
                        if (resultIdx < scalarResultFrames->at(requestedTimesteps[tIdx]).size())
                        {
                            socketStream << scalarResultFrames->at(requestedTimesteps[tIdx])[resultIdx];
                        }
                        else
                        {
                            socketStream << HUGE_VAL;
                        }
                    }
                }
            }
#if 0
            // This aproach is faster but does not handle coarsening
            size_t  timestepResultCount = scalarResultFrames->front().size();
            quint64 timestepByteCount = (quint64)(timestepResultCount*sizeof(double));
            socketStream << timestepByteCount ;

            // Then write the data.

            for (size_t tIdx = 0; tIdx < requestedTimesteps.size(); ++tIdx)
            {
#if 1 // Write data as raw bytes, fast but does not handle byteswapping
                server->currentClient()->write((const char *)scalarResultFrames->at(requestedTimesteps[tIdx]).data(), timestepByteCount); // Raw print of data. Fast but no platform conversion
#else  // Write data using QDataStream, does byteswapping for us. Must use QDataStream on client as well
                for (size_t cIdx = 0; cIdx < scalarResultFrames->at(requestedTimesteps[tIdx]).size(); ++cIdx)
                {
                    socketStream << scalarResultFrames->at(tIdx)[cIdx];
                }
#endif
            }
#endif
        }

        return true;
    }
};

static bool RiaGetActiveCellProperty_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetActiveCellProperty>(RiaGetActiveCellProperty::commandName());
