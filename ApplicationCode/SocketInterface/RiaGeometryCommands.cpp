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

#include "RiaSocketCommand.h"

#include "RiaApplication.h"
#include "RiaPreferences.h"
#include "RiaSocketServer.h"
#include "RiaSocketTools.h"

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
/// Convert internal ResInsight representation of cells with negative depth to positive depth.
//--------------------------------------------------------------------------------------------------
static inline void convertVec3dToPositiveDepth(cvf::Vec3d* vec)
{
    double& z = vec->z();
    z *= -1;
}

//--------------------------------------------------------------------------------------------------
/// Retrieve a cell corner where the depth is represented as negative converted to positive depth.
//--------------------------------------------------------------------------------------------------
static inline double getCellCornerWithPositiveDepth(const cvf::Vec3d *cornerVerts, size_t cornerIndexMapping, int coordIdx)
{
    if (coordIdx == 2)
    {
        // Z-value aka depth
        return -1 * cornerVerts[cornerIndexMapping][coordIdx];
    }
    else
    {
        return cornerVerts[cornerIndexMapping][coordIdx];
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetCellCenters : public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetCellCenters"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        size_t argGridIndex = args[2].toUInt();

        if (!rimCase || !rimCase->eclipseCaseData() || (argGridIndex >= rimCase->eclipseCaseData()->gridCount()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0;
            return true;
        }

        RigGridBase* rigGrid = rimCase->eclipseCaseData()->grid(argGridIndex);

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

        size_t blockByteCount = cellCount * sizeof(double);
        std::vector<double> doubleValues(blockByteCount);

        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            quint64 valueIndex = 0;

            for (size_t k = 0; k < cellCountK; k++)
            {
                for (size_t j = 0; j < cellCountJ; j++)
                {
                    for (size_t i = 0; i < cellCountI; i++)
                    {
                        size_t gridLocalCellIndex = rigGrid->cellIndexFromIJK(i, j, k);
                        cvf::Vec3d center = rigGrid->cell(gridLocalCellIndex).center();

                        convertVec3dToPositiveDepth(&center);

                        doubleValues[valueIndex++] = center[coordIdx];
                    }
                }
            }

            CVF_ASSERT(valueIndex == cellCount);

            RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)doubleValues.data(), blockByteCount);
        }

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
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);

        QString porosityModelName;
        porosityModelName = args[2];

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName.toUpper() == "FRACTURE")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        if (!rimCase || !rimCase->eclipseCaseData())
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return true;
        }

        RigActiveCellInfo* actCellInfo = rimCase->eclipseCaseData()->activeCellInfo(porosityModelEnum);
        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();

        size_t activeCellCount = actCellInfo->reservoirActiveCellCount();
        size_t doubleValueCount = activeCellCount * 3;

        socketStream << (quint64)activeCellCount;
        quint64 byteCount = doubleValueCount * sizeof(double);
        socketStream << byteCount;


        // This structure is supposed to be received by Octave using a NDArray. The ordering of this loop is
        // defined by the ordering of the receiving NDArray
        //
        // See riGetActiveCellCenters
        //
        //  dim_vector dv;
        //  dv.resize(2);
        //  dv(0) = coordCount;
        //  dv(1) = 3;

        size_t blockByteCount = activeCellCount * sizeof(double);
        std::vector<double> doubleValues(blockByteCount);

        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            quint64 valueIndex = 0;

            for (size_t reservoirCellIndex = 0; reservoirCellIndex < mainGrid->globalCellArray().size(); reservoirCellIndex++)
            {
                if (!actCellInfo->isActive(reservoirCellIndex)) continue;

                cvf::Vec3d center = mainGrid->globalCellArray()[reservoirCellIndex].center();

                convertVec3dToPositiveDepth(&center);

                doubleValues[valueIndex++] = center[coordIdx];
            }

            CVF_ASSERT(valueIndex == activeCellCount);
            RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)doubleValues.data(), blockByteCount);
        }

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
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        size_t argGridIndex = args[2].toUInt();

        if (!rimCase || !rimCase->eclipseCaseData() || (argGridIndex >= rimCase->eclipseCaseData()->gridCount()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0 << (quint64)0;
            return true;
        }

        RigGridBase* rigGrid = rimCase->eclipseCaseData()->grid(argGridIndex);

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

        cvf::Vec3d cornerVerts[8];
        size_t blockByteCount = cellCount * sizeof(double);
        std::vector<double> doubleValues(blockByteCount);

        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t cornerIdx = 0; cornerIdx < 8; cornerIdx++)
            {
                size_t cornerIndexMapping = cellCornerMappingEclipse[cornerIdx];

                quint64 valueIndex = 0;

                for (size_t k = 0; k < cellCountK; k++)
                {
                    for (size_t j = 0; j < cellCountJ; j++)
                    {
                        for (size_t i = 0; i < cellCountI; i++)
                        {
                            size_t gridLocalCellIndex = rigGrid->cellIndexFromIJK(i, j, k);
                            rigGrid->cellCornerVertices(gridLocalCellIndex, cornerVerts);

                            doubleValues[valueIndex++] = getCellCornerWithPositiveDepth(cornerVerts, cornerIndexMapping, coordIdx);
                        }
                    }
                }

                CVF_ASSERT(valueIndex == cellCount);

                RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)doubleValues.data(), blockByteCount);
            }
        }

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
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);

        QString porosityModelName;
        porosityModelName = args[2];

        RifReaderInterface::PorosityModelResultType porosityModelEnum = RifReaderInterface::MATRIX_RESULTS;
        if (porosityModelName.toUpper() == "FRACTURE")
        {
            porosityModelEnum = RifReaderInterface::FRACTURE_RESULTS;
        }

        if (!rimCase || !rimCase->eclipseCaseData() )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return true;
        }

        RigActiveCellInfo* actCellInfo = rimCase->eclipseCaseData()->activeCellInfo(porosityModelEnum);
        RigMainGrid* mainGrid = rimCase->eclipseCaseData()->mainGrid();

        size_t activeCellCount = actCellInfo->reservoirActiveCellCount();
        size_t doubleValueCount = activeCellCount * 3 * 8;

        socketStream << (quint64)activeCellCount;
        quint64 byteCount = doubleValueCount * sizeof(double);
        socketStream << byteCount;

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

        cvf::Vec3d cornerVerts[8];
        size_t blockByteCount = activeCellCount * sizeof(double);
        std::vector<double> doubleValues(blockByteCount);

        for (int coordIdx = 0; coordIdx < 3; coordIdx++)
        {
            for (size_t cornerIdx = 0; cornerIdx < 8; cornerIdx++)
            {
                size_t cornerIndexMapping = cellCornerMappingEclipse[cornerIdx];

                quint64 valueIndex = 0;

                for (size_t reservoirCellIndex = 0; reservoirCellIndex < mainGrid->globalCellArray().size(); reservoirCellIndex++)
                {
                    if (!actCellInfo->isActive(reservoirCellIndex)) continue;

                    mainGrid->cellCornerVertices(reservoirCellIndex, cornerVerts);

                    doubleValues[valueIndex++] = getCellCornerWithPositiveDepth(cornerVerts, cornerIndexMapping, coordIdx);
                }

                CVF_ASSERT(valueIndex == activeCellCount);

                RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)doubleValues.data(), blockByteCount);
            }
        }

        return true;
    }

};

static bool RiaGetActiveCellCorners_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetActiveCellCorners>(RiaGetActiveCellCorners::commandName());
