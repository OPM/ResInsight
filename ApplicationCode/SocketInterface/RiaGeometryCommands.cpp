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

#include <QTcpSocket>


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetCellCenters : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCellCenters"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;
        size_t argGridIndex = 0;

        if (args.size() == 2)
        {
            argGridIndex = args[1].toInt();
        }
        else if (args.size() == 3)
        {
            argCaseGroupId = args[1].toInt();
            argGridIndex = args[2].toUInt();
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->reservoirData() || (argGridIndex >= rimCase->reservoirData()->gridCount()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0;
            return true;
        }

        RigGridBase* rigGrid = rimCase->reservoirData()->grid(argGridIndex);

        quint64 cellCount  = (quint64)rigGrid->cellCount();
        quint64 cellCountI = (quint64)rigGrid->cellCountI();
        quint64 cellCountJ = (quint64)rigGrid->cellCountJ();
        quint64 cellCountK = (quint64)rigGrid->cellCountK();

        socketStream << cellCount;
        socketStream << cellCountI;
        socketStream << cellCountJ;
        socketStream << cellCountK;

        size_t doubleValueCount = cellCount * 3;
        quint64 byteCount = doubleValueCount * sizeof(double);
        socketStream << byteCount;

        // This structure is supposed to be received by Octave using a NDArray. The ordering of this loop is
        // defined by the ordering of the receiving NDArray
        //
        // See riGetCellCenters
        //
        //  dim_vector dv;
        //  dv.resize(4);
        //  dv(0) = cellCountI;
        //  dv(1) = cellCountJ;
        //  dv(2) = cellCountK;
        //  dv(3) = 3;

        std::vector<double> cellCenterValues(doubleValueCount);
        cvf::Vec3d cornerVerts[8];
        quint64 coordCount = 0;
        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t k = 0; k < cellCountK; k++)
            {
                for (size_t j = 0; j < cellCountJ; j++)
                {
                    for (size_t i = 0; i < cellCountI; i++)
                    {
                        size_t localCellIdx = rigGrid->cellIndexFromIJK(i, j, k);
                        cvf::Vec3d center = rigGrid->cell(localCellIdx).center();

                        cellCenterValues[coordCount++] = center[coordIdx];
                    }
                }
            }
        }

        CVF_ASSERT(coordCount == doubleValueCount);

        server->currentClient()->write((const char *)cellCenterValues.data(), byteCount);

        return true;
    }
};

static bool RiaGetCellCenters_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetCellCenters>(RiaGetCellCenters::commandName());


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetActiveCellCenters : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetActiveCellCenters"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;
        QString porosityModelName;

        if (args.size() == 2)
        {
            bool numberConversionOk = false;
            int tmpValue = args[1].toInt(&numberConversionOk);
            if (numberConversionOk)
            {
                argCaseGroupId = tmpValue;
            }
            else
            {
                porosityModelName = args[1];
            }
        }
        else if (args.size() == 3)
        {
            bool numberConversionOk = false;
            int tmpValue = args[1].toInt(&numberConversionOk);
            if (numberConversionOk)
            {
                argCaseGroupId = args[1].toUInt();
                porosityModelName = args[2];
            }
            else
            {
                argCaseGroupId = args[2].toUInt();
                porosityModelName = args[1];
            }
        }

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName.toUpper() == "FRACTURE")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->reservoirData())
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return true;
        }

        RigActiveCellInfo* actCellInfo = rimCase->reservoirData()->activeCellInfo(porosityModelEnum);
        RigMainGrid* mainGrid = rimCase->reservoirData()->mainGrid();

        size_t activeCellCount = actCellInfo->globalActiveCellCount();
        size_t doubleValueCount = activeCellCount * 3;

        // This structure is supposed to be received by Octave using a NDArray. The ordering of this loop is
        // defined by the ordering of the receiving NDArray
        //
        // See riGetActiveCellCenters
        //
        //  dim_vector dv;
        //  dv.resize(2);
        //  dv(0) = coordCount;
        //  dv(1) = 3;

        std::vector<double> cellCenterValues(doubleValueCount);
        quint64 coordCount = 0;
        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t globalCellIdx = 0; globalCellIdx < mainGrid->cells().size(); globalCellIdx++)
            {
                if (!actCellInfo->isActive(globalCellIdx)) continue;

                cvf::Vec3d center = mainGrid->cells()[globalCellIdx].center();

                cellCenterValues[coordCount++] = center[coordIdx];
            }
        }

        CVF_ASSERT(coordCount == doubleValueCount);

        socketStream << (quint64)activeCellCount;
        quint64 byteCount = doubleValueCount * sizeof(double);
        socketStream << byteCount;

        server->currentClient()->write((const char *)cellCenterValues.data(), byteCount);

        return true;
    }

};

static bool RiaGetActiveCellCenters_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetActiveCellCenters>(RiaGetActiveCellCenters::commandName());


// NB: Match this mapping with the mapping in RifReaderEclipseOutput.cpp 
static const size_t cellCornerMappingEclipse[8] = { 0, 1, 3, 2, 4, 5, 7, 6 };

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetCellCorners : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCellCorners"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;
        size_t argGridIndex = 0;

        if (args.size() == 2)
        {
            argGridIndex = args[1].toInt();
        }
        else if (args.size() == 3)
        {
            argCaseGroupId = args[1].toInt();
            argGridIndex = args[2].toUInt();
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->reservoirData() || (argGridIndex >= rimCase->reservoirData()->gridCount()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0;
            return true;
        }

        RigGridBase* rigGrid = rimCase->reservoirData()->grid(argGridIndex);

        quint64 cellCount  = (quint64)rigGrid->cellCount();
        quint64 cellCountI = (quint64)rigGrid->cellCountI();
        quint64 cellCountJ = (quint64)rigGrid->cellCountJ();
        quint64 cellCountK = (quint64)rigGrid->cellCountK();

        size_t doubleValueCount = cellCount * 3 * 8;
        quint64 byteCount = doubleValueCount * sizeof(double);

        socketStream << cellCount;
        socketStream << cellCountI;
        socketStream << cellCountJ;
        socketStream << cellCountK;
        socketStream << byteCount;

        // This structure is supposed to be received by Octave using a NDArray. The ordering of this loop is
        // defined by the ordering of the receiving NDArray
        //
        // See riGetCellCorners
        //
        //  dim_vector dv;
        //  dv.resize(5);
        //  dv(0) = cellCountI;
        //  dv(1) = cellCountJ;
        //  dv(2) = cellCountK;
        //  dv(3) = 8;
        //  dv(4) = 3;

        std::vector<double> cellCornerValues(doubleValueCount);
        cvf::Vec3d cornerVerts[8];
        quint64 coordCount = 0;
        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t cornerIdx = 0; cornerIdx < 8; cornerIdx++)
            {
                size_t cornerIndexMapping = cellCornerMappingEclipse[cornerIdx];

                for (size_t k = 0; k < cellCountK; k++)
                {
                    for (size_t j = 0; j < cellCountJ; j++)
                    {
                        for (size_t i = 0; i < cellCountI; i++)
                        {
                            size_t localCellIdx = rigGrid->cellIndexFromIJK(i, j, k);
                            rigGrid->cellCornerVertices(localCellIdx, cornerVerts);

                            cellCornerValues[coordCount++] = cornerVerts[cornerIndexMapping][coordIdx];
                        }
                    }
                }
            }
        }

        server->currentClient()->write((const char *)cellCornerValues.data(), byteCount);

        return true;
    }
};

static bool RiaGetCellCorners_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetCellCorners>(RiaGetCellCorners::commandName());



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetActiveCellCorners : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetActiveCellCorners"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;
        QString porosityModelName;

        if (args.size() == 2)
        {
            bool numberConversionOk = false;
            int tmpValue = args[1].toInt(&numberConversionOk);
            if (numberConversionOk)
            {
                argCaseGroupId = tmpValue;
            }
            else
            {
                porosityModelName = args[1];
            }
        }
        else if (args.size() == 3)
        {
            bool numberConversionOk = false;
            int tmpValue = args[1].toInt(&numberConversionOk);
            if (numberConversionOk)
            {
                argCaseGroupId = args[1].toUInt();
                porosityModelName = args[2];
            }
            else
            {
                argCaseGroupId = args[2].toUInt();
                porosityModelName = args[1];
            }
        }

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName.toUpper() == "FRACTURE")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->reservoirData() )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return true;
        }

        RigActiveCellInfo* actCellInfo = rimCase->reservoirData()->activeCellInfo(porosityModelEnum);
        RigMainGrid* mainGrid = rimCase->reservoirData()->mainGrid();

        size_t activeCellCount = actCellInfo->globalActiveCellCount();
        size_t doubleValueCount = activeCellCount * 3 * 8;

        // This structure is supposed to be received by Octave using a NDArray. The ordering of this loop is
        // defined by the ordering of the receiving NDArray
        //
        // See riGetCellCorners
        //
        //  dim_vector dv;
        //  dv.resize(3);
        //  dv(0) = coordCount;
        //  dv(1) = 8;
        //  dv(2) = 3;

        std::vector<double> cellCornerValues(doubleValueCount);
        cvf::Vec3d cornerVerts[8];
        quint64 coordCount = 0;
        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t cornerIdx = 0; cornerIdx < 8; cornerIdx++)
            {
                size_t cornerIndexMapping = cellCornerMappingEclipse[cornerIdx];

                for (size_t globalCellIdx = 0; globalCellIdx < mainGrid->cells().size(); globalCellIdx++)
                {
                    if (!actCellInfo->isActive(globalCellIdx)) continue;

                    mainGrid->cellCornerVertices(globalCellIdx, cornerVerts);

                    cellCornerValues[coordCount++] = cornerVerts[cornerIndexMapping][coordIdx];
                }
            }
        }

        socketStream << (quint64)activeCellCount;
        quint64 byteCount = doubleValueCount * sizeof(double);
        socketStream << byteCount;

        server->currentClient()->write((const char *)cellCornerValues.data(), byteCount);

        return true;
    }

};

static bool RiaGetActiveCellCorners_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetActiveCellCorners>(RiaGetActiveCellCorners::commandName());
