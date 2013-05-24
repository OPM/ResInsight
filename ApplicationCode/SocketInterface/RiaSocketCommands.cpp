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
#include "RiaSocketCommand.h"

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




class RiaGetCoarseningInfo : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCoarseningInfo"); }

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

        // Write data back to octave: I1, I2, J1, J2, K1, K2 for all coarsening boxes

        if (rimCase && rimCase->reservoirData() && rimCase->reservoirData()->mainGrid())
        {
            size_t globalCoarseningBoxCount = 0;

            for (size_t gridIdx = 0; gridIdx < rimCase->reservoirData()->gridCount(); gridIdx++)
            {
                RigGridBase* grid = rimCase->reservoirData()->grid(gridIdx);

                size_t localCoarseningBoxCount = grid->coarseningBoxCount();
                globalCoarseningBoxCount += localCoarseningBoxCount;
            }

            quint64 byteCount = globalCoarseningBoxCount * 6 * sizeof(qint32);
            socketStream << byteCount;

            for (size_t gridIdx = 0; gridIdx < rimCase->reservoirData()->gridCount(); gridIdx++)
            {
                RigGridBase* grid = rimCase->reservoirData()->grid(gridIdx);

                size_t localCoarseningBoxCount = grid->coarseningBoxCount();
                for (size_t boxIdx = 0; boxIdx < localCoarseningBoxCount; boxIdx++)
                {
                    size_t i1, i2, j1, j2, k1, k2;
                    grid->coarseningBox(boxIdx, &i1, &i2, &j1, &j2, &k1, &k2);

                    // Write 1-based coordinates for coarsening box
                    socketStream << static_cast<qint32>(i1 + 1);
                    socketStream << static_cast<qint32>(i2 + 1);
                    socketStream << static_cast<qint32>(j1 + 1);
                    socketStream << static_cast<qint32>(j2 + 1);
                    socketStream << static_cast<qint32>(k1 + 1);
                    socketStream << static_cast<qint32>(k2 + 1);
                }
            }
        }

        return true;
    }
};

static bool RiaGetCoarseningInfo_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetCoarseningInfo>(RiaGetCoarseningInfo::commandName());


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
        for (size_t coordIdx = 0; coordIdx < 3; coordIdx++)
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


class RiaGetActiveCellCenters : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetActiveCellCenters"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;
        size_t argGridIndex = 0;
        QString porosityModelName;

        if (args.size() == 2)
        {
            argGridIndex = args[1].toInt();
        }
        else if (args.size() == 3)
        {
            bool numberConversionOk = false;
            int tmpValue = args[2].toInt(&numberConversionOk);
            if (numberConversionOk)
            {
                // Two arguments, caseID and gridIndex
                argCaseGroupId = args[1].toInt();
                argGridIndex = args[2].toUInt();
            }
            else
            {
                // Two arguments, gridIndex and porosity model
                argGridIndex = args[1].toUInt();
                porosityModelName = args[2];
            }
        }
        else if (args.size() > 3)
        {
            // Two arguments, caseID and gridIndex
            argCaseGroupId = args[1].toInt();
            argGridIndex = args[2].toUInt();
            porosityModelName = args[3];
        }

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName.toUpper() == "FRACTURE")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->reservoirData() || (argGridIndex >= rimCase->reservoirData()->gridCount()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return true;
        }

        RigActiveCellInfo* actCellInfo = rimCase->reservoirData()->activeCellInfo(porosityModelEnum);

        RigGridBase* rigGrid = rimCase->reservoirData()->grid(argGridIndex);

        quint64 cellCountI = (quint64)rigGrid->cellCountI();
        quint64 cellCountJ = (quint64)rigGrid->cellCountJ();
        quint64 cellCountK = (quint64)rigGrid->cellCountK();

        size_t activeCellCount = 0;
        actCellInfo->gridActiveCellCounts(argGridIndex, activeCellCount);
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
        for (size_t coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t k = 0; k < cellCountK; k++)
            {
                for (size_t j = 0; j < cellCountJ; j++)
                {
                    for (size_t i = 0; i < cellCountI; i++)
                    {
                        size_t localCellIdx = rigGrid->cellIndexFromIJK(i, j, k);
                        size_t globalCellIdx = rigGrid->globalGridCellIndex(localCellIdx);

                        if (!actCellInfo->isActive(globalCellIdx)) continue;

                        cvf::Vec3d center = rigGrid->cell(localCellIdx).center();

                        cellCenterValues[coordCount++] = center[coordIdx];
                    }
                }
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
        for (size_t coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t cornerIdx = 0; cornerIdx < 8; cornerIdx++)
            {
                for (size_t k = 0; k < cellCountK; k++)
                {
                    for (size_t j = 0; j < cellCountJ; j++)
                    {
                        for (size_t i = 0; i < cellCountI; i++)
                        {
                            size_t localCellIdx = rigGrid->cellIndexFromIJK(i, j, k);
                            rigGrid->cellCornerVertices(localCellIdx, cornerVerts);

                            cellCornerValues[coordCount++] = cornerVerts[cornerIdx][coordIdx];
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


class RiaGetActiveCellCorners : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetActiveCellCorners"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        int argCaseGroupId = -1;
        size_t argGridIndex = 0;
        QString porosityModelName;

        if (args.size() == 2)
        {
            argGridIndex = args[1].toInt();
        }
        else if (args.size() == 3)
        {
            bool numberConversionOk = false;
            int tmpValue = args[2].toInt(&numberConversionOk);
            if (numberConversionOk)
            {
                // Two arguments, caseID and gridIndex
                argCaseGroupId = args[1].toInt();
                argGridIndex = args[2].toUInt();
            }
            else
            {
                // Two arguments, gridIndex and porosity model
                argGridIndex = args[1].toUInt();
                porosityModelName = args[2];
            }
        }
        else if (args.size() > 3)
        {
            // Two arguments, caseID and gridIndex
            argCaseGroupId = args[1].toInt();
            argGridIndex = args[2].toUInt();
            porosityModelName = args[3];
        }

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName.toUpper() == "FRACTURE")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        RimCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->reservoirData() || (argGridIndex >= rimCase->reservoirData()->gridCount()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return true;
        }

        RigActiveCellInfo* actCellInfo = rimCase->reservoirData()->activeCellInfo(porosityModelEnum);
        RigGridBase* rigGrid = rimCase->reservoirData()->grid(argGridIndex);

        quint64 cellCountI = (quint64)rigGrid->cellCountI();
        quint64 cellCountJ = (quint64)rigGrid->cellCountJ();
        quint64 cellCountK = (quint64)rigGrid->cellCountK();

        size_t activeCellCount = 0;
        actCellInfo->gridActiveCellCounts(argGridIndex, activeCellCount);
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
        for (size_t coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t cornerIdx = 0; cornerIdx < 8; cornerIdx++)
            {
                for (size_t k = 0; k < cellCountK; k++)
                {
                    for (size_t j = 0; j < cellCountJ; j++)
                    {
                        for (size_t i = 0; i < cellCountI; i++)
                        {
                            size_t localCellIdx = rigGrid->cellIndexFromIJK(i, j, k);
                            size_t globalCellIdx = rigGrid->globalGridCellIndex(localCellIdx);

                            if (!actCellInfo->isActive(globalCellIdx)) continue;

                            rigGrid->cellCornerVertices(localCellIdx, cornerVerts);

                            cellCornerValues[coordCount++] = cornerVerts[cornerIdx][coordIdx];
                        }
                    }
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
