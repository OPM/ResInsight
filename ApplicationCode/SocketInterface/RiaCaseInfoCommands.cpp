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

#include "RiaSocketServer.h"
#include "RiaSocketTools.h"
#include "RiaApplication.h"
#include "RiaPreferences.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"
#include "RigGeoMechCaseData.h"
#include "RigFemPartCollection.h"
#include "RigFemPart.h"
#include "RigFemPartGrid.h"

#include "Rim3dOverlayInfoConfig.h"
#include "RimCellEdgeColors.h"
#include "RimCellRangeFilterCollection.h"
#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipsePropertyFilterCollection.h"
#include "RimEclipseView.h"
#include "RimEclipseWellCollection.h"
#include "RimReservoirCellResultsStorage.h"
#include "RimGeoMechCase.h"
#include "RimGeoMechView.h"

#include "RiuSelectionManager.h"

#include <QTcpSocket>




//--------------------------------------------------------------------------------------------------
/// OBSOLETE, to be deleted
//--------------------------------------------------------------------------------------------------
class RiaGetMainGridDimensions: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetMainGridDimensions"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {

        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        if (!rimCase) return true;

        // Write data back to octave: I, J, K dimensions

        size_t iCount = 0;
        size_t jCount = 0;
        size_t kCount = 0;

        if (rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid())
        {
            iCount = rimCase->eclipseCaseData()->mainGrid()->cellCountI();
            jCount = rimCase->eclipseCaseData()->mainGrid()->cellCountJ();
            kCount = rimCase->eclipseCaseData()->mainGrid()->cellCountK();
        }

        socketStream << (quint64)iCount << (quint64)jCount << (quint64)kCount;

        return true;
    }
};

static bool RiaGetMainGridDimensions_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetMainGridDimensions>(RiaGetMainGridDimensions::commandName());



//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetActiveCellInfo: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetActiveCellInfo"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>&  args, QDataStream& socketStream)
    {
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        if (!rimCase) return true;

        RiaDefines::PorosityModelType porosityModel = RiaDefines::MATRIX_MODEL;

        if (args.size() > 2)
        {
            QString prorosityModelString = args[2];
            if (prorosityModelString.toUpper() == "FRACTURE")
            {
                porosityModel = RiaDefines::FRACTURE_MODEL;
            }
        }

        // Write data back to octave: columnCount, bytesPrTimestep, GridNr I J K ParentGridNr PI PJ PK CoarseBoxIdx

        caf::FixedArray<std::vector<qint32>, 9> activeCellInfo;
        if (!(rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid()) )
        {
            // No data available
            socketStream << (quint64)0 << (quint64)0 ;
            return true;
        }

        calculateMatrixModelActiveCellInfo(rimCase, porosityModel,
            activeCellInfo[0],
            activeCellInfo[1],
            activeCellInfo[2],
            activeCellInfo[3],
            activeCellInfo[4],
            activeCellInfo[5],
            activeCellInfo[6],
            activeCellInfo[7],
            activeCellInfo[8]);

        // First write column count
        quint64 columnCount = (quint64)9;
        socketStream << columnCount;

        // then the byte-size of the size of one column
        size_t  timestepResultCount = activeCellInfo[0].size();
        quint64 timestepByteCount = (quint64)(timestepResultCount*sizeof(qint32));
        socketStream << timestepByteCount;

        for (size_t tIdx = 0; tIdx < columnCount; ++tIdx)
        {
            RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)activeCellInfo[tIdx].data(), timestepByteCount);
        }

        return true;
    }

    static void calculateMatrixModelActiveCellInfo(RimEclipseCase* reservoirCase, RiaDefines::PorosityModelType porosityModel, std::vector<qint32>& gridNumber, std::vector<qint32>& cellI, std::vector<qint32>& cellJ, std::vector<qint32>& cellK, std::vector<qint32>& parentGridNumber, std::vector<qint32>& hostCellI, std::vector<qint32>& hostCellJ, std::vector<qint32>& hostCellK, std::vector<qint32>& globalCoarseningBoxIdx)
    {
        gridNumber.clear();
        cellI.clear();
        cellJ.clear();
        cellK.clear();
        parentGridNumber.clear();
        hostCellI.clear();
        hostCellJ.clear();
        hostCellK.clear();
        globalCoarseningBoxIdx.clear();

        if (!reservoirCase || !reservoirCase->eclipseCaseData() || !reservoirCase->eclipseCaseData()->mainGrid())
        {
            return;
        }

        RigActiveCellInfo* actCellInfo = reservoirCase->eclipseCaseData()->activeCellInfo(porosityModel);
        size_t numMatrixModelActiveCells = actCellInfo->reservoirActiveCellCount();

        gridNumber.reserve(numMatrixModelActiveCells);
        cellI.reserve(numMatrixModelActiveCells);
        cellJ.reserve(numMatrixModelActiveCells);
        cellK.reserve(numMatrixModelActiveCells);
        parentGridNumber.reserve(numMatrixModelActiveCells);
        hostCellI.reserve(numMatrixModelActiveCells);
        hostCellJ.reserve(numMatrixModelActiveCells);
        hostCellK.reserve(numMatrixModelActiveCells);
        globalCoarseningBoxIdx.reserve(numMatrixModelActiveCells);

        const std::vector<RigCell>& reservoirCells = reservoirCase->eclipseCaseData()->mainGrid()->globalCellArray();


        std::vector<size_t> globalCoarseningBoxIndexStart;
        {
            size_t globalCoarseningBoxCount = 0;

            for (size_t gridIdx = 0; gridIdx < reservoirCase->eclipseCaseData()->gridCount(); gridIdx++)
            {
                globalCoarseningBoxIndexStart.push_back(globalCoarseningBoxCount);

                RigGridBase* grid = reservoirCase->eclipseCaseData()->grid(gridIdx);

                size_t localCoarseningBoxCount = grid->coarseningBoxCount();
                globalCoarseningBoxCount += localCoarseningBoxCount;
            }

        }


        for (size_t cIdx = 0; cIdx < reservoirCells.size(); ++cIdx)
        {
            if (actCellInfo->isActive(cIdx))
            {
                RigGridBase* grid = reservoirCells[cIdx].hostGrid();
                CVF_ASSERT(grid != NULL);
                size_t cellIndex = reservoirCells[cIdx].gridLocalCellIndex();

                size_t i, j, k;
                grid->ijkFromCellIndex(cellIndex, &i, &j, &k);

                size_t pi, pj, pk;
                RigGridBase* parentGrid = NULL;

                if (grid->isMainGrid())
                {
                    pi = i;
                    pj = j;
                    pk = k;
                    parentGrid = grid;
                }
                else
                {
                    size_t parentCellIdx = reservoirCells[cIdx].parentCellIndex();
                    parentGrid = (static_cast<RigLocalGrid*>(grid))->parentGrid();
                    CVF_ASSERT(parentGrid != NULL);
                    parentGrid->ijkFromCellIndex(parentCellIdx, &pi, &pj, &pk);
                }

                gridNumber.push_back(static_cast<qint32>(grid->gridIndex()));
                cellI.push_back(static_cast<qint32>(i + 1));        // NB: 1-based index in Octave
                cellJ.push_back(static_cast<qint32>(j + 1));        // NB: 1-based index in Octave
                cellK.push_back(static_cast<qint32>(k + 1));        // NB: 1-based index in Octave

                parentGridNumber.push_back(static_cast<qint32>(parentGrid->gridIndex()));
                hostCellI.push_back(static_cast<qint32>(pi + 1));   // NB: 1-based index in Octave
                hostCellJ.push_back(static_cast<qint32>(pj + 1));   // NB: 1-based index in Octave
                hostCellK.push_back(static_cast<qint32>(pk + 1));   // NB: 1-based index in Octave

                size_t coarseningIdx = reservoirCells[cIdx].coarseningBoxIndex();
                if (coarseningIdx != cvf::UNDEFINED_SIZE_T)
                {
                    size_t globalCoarseningIdx = globalCoarseningBoxIndexStart[grid->gridIndex()] + coarseningIdx;

                    globalCoarseningBoxIdx.push_back(static_cast<qint32>(globalCoarseningIdx + 1)); // NB: 1-based index in Octave
                }
                else
                {
                    globalCoarseningBoxIdx.push_back(-1);
                }
            }
        }
    }
};

static bool RiaGetActiveCellInfo_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetActiveCellInfo>(RiaGetActiveCellInfo::commandName());


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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

        RimEclipseCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->eclipseCaseData() || !rimCase->eclipseCaseData()->mainGrid())
        {
            quint64 byteCount = 0;

            socketStream << byteCount;

            return true;
        }

        // Write data back to octave: I1, I2, J1, J2, K1, K2 for all coarsening boxes

        if (rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid())
        {
            size_t globalCoarseningBoxCount = 0;

            for (size_t gridIdx = 0; gridIdx < rimCase->eclipseCaseData()->gridCount(); gridIdx++)
            {
                RigGridBase* grid = rimCase->eclipseCaseData()->grid(gridIdx);

                size_t localCoarseningBoxCount = grid->coarseningBoxCount();
                globalCoarseningBoxCount += localCoarseningBoxCount;
            }

            quint64 byteCount = globalCoarseningBoxCount * 6 * sizeof(qint32);
            socketStream << byteCount;

            for (size_t gridIdx = 0; gridIdx < rimCase->eclipseCaseData()->gridCount(); gridIdx++)
            {
                RigGridBase* grid = rimCase->eclipseCaseData()->grid(gridIdx);

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


//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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

        RimEclipseCase* rimCase = server->findReservoir(argCaseGroupId);
        if (!rimCase || !rimCase->eclipseCaseData() || !rimCase->eclipseCaseData()->mainGrid())
        {
            quint64 byteCount = 0;

            socketStream << byteCount;

            return true;
        }

        // Write data back to octave: I, J, K dimensions


        if (rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid())
        {
            std::vector<RigGridBase*> grids;
            rimCase->eclipseCaseData()->allGrids(&grids);

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



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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

        RimEclipseCase* rimCase = server->findReservoir(argCaseGroupId);

        bool canFetchData = true;

        if (!rimCase || !rimCase->eclipseCaseData())
        {
            canFetchData = false;
        }

        size_t scalarIndexWithMaxTimeStepCount = cvf::UNDEFINED_SIZE_T;
        if (rimCase && rimCase->eclipseCaseData())
        {
            rimCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->maxTimeStepCount(&scalarIndexWithMaxTimeStepCount);
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

        std::vector<QDateTime> timeStepDates = rimCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->timeStepDates(scalarIndexWithMaxTimeStepCount);

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



//--------------------------------------------------------------------------------------------------
/// 
//--------------------------------------------------------------------------------------------------
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

        RimEclipseCase* rimCase = server->findReservoir(argCaseGroupId);

        bool canFetchData = true;

        if (!rimCase || !rimCase->eclipseCaseData())
        {
            canFetchData = false;
        }

        size_t scalarIndexWithMaxTimeStepCount = cvf::UNDEFINED_SIZE_T;
        if (rimCase && rimCase->eclipseCaseData())
        {
            rimCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->maxTimeStepCount(&scalarIndexWithMaxTimeStepCount);
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

        std::vector<double> daysSinceSimulationStart = rimCase->eclipseCaseData()->results(RiaDefines::MATRIX_MODEL)->daysSinceSimulationStart(scalarIndexWithMaxTimeStepCount);

        quint64 timeStepCount = daysSinceSimulationStart.size();
        quint64 byteCount = sizeof(quint64) + timeStepCount * sizeof(qint32);

        socketStream << byteCount;
        socketStream << timeStepCount;

        for (double day : daysSinceSimulationStart)
        {
            socketStream << day;
        }

        return true;
    }

};

static bool RiaGetTimeStepDays_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetTimeStepDays>(RiaGetTimeStepDays::commandName());

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
class RiaGetSelectedCells: public RiaSocketCommand
{
public:
    static QString commandName () { return QString("GetSelectedCells"); }

    virtual bool interpretCommand(RiaSocketServer* server, const QList<QByteArray>& args, QDataStream& socketStream)
    {
        // findCaseFromArgs only returns RimEclipseCase, so geomech cases are not supported because of this.
        // The rest of the function supports geomech cases, so using a findCaseFromArgs that supports geomech
        // cases is the only change needed to add geomech support.
        RimEclipseCase* rimCase = RiaSocketTools::findCaseFromArgs(server, args);
        if (!rimCase) return true;

        // Write data back to octave: column count, bytes per column, caseId, gridNumber, cellI, cellJ, cellK

        std::array<std::vector<qint32>, 5> selectedCellInfo;
        getSelectedCells(rimCase,
                         selectedCellInfo[0],
                         selectedCellInfo[1],
                         selectedCellInfo[2],
                         selectedCellInfo[3],
                         selectedCellInfo[4]);

        // First write column count
        quint64 columnCount = 5;
        socketStream << columnCount;

        // then the byte-size of the size of one column
        quint64 columnByteCount = (quint64)(selectedCellInfo[0].size()*sizeof(qint32));
        socketStream << columnByteCount;

        // Write back table data
        for (size_t tIdx = 0; tIdx < columnCount; ++tIdx)
        {
            RiaSocketTools::writeBlockData(server, server->currentClient(), (const char *)selectedCellInfo[tIdx].data(), columnByteCount);
        }

        return true;
    }

    static void getSelectedCells(const RimCase* reservoirCase,
                                 std::vector<qint32>& caseNumber,
                                 std::vector<qint32>& gridNumber,
                                 std::vector<qint32>& cellI,
                                 std::vector<qint32>& cellJ,
                                 std::vector<qint32>& cellK)
    {
        std::vector<RiuSelectionItem*> items;
        RiuSelectionManager::instance()->selectedItems(items);

        for (const RiuSelectionItem* item : items)
        {
            size_t i, j, k;
            size_t gridIndex;
            int caseId;
            if (item->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT)
            {
                const RiuEclipseSelectionItem* eclipseItem = static_cast<const RiuEclipseSelectionItem*>(item);

                eclipseItem->m_view->eclipseCase()->eclipseCaseData()->grid(eclipseItem->m_gridIndex)->ijkFromCellIndex(eclipseItem->m_cellIndex, &i, &j, &k);
                gridIndex = eclipseItem->m_gridIndex;
                caseId = eclipseItem->m_view->eclipseCase()->caseId;
            }
            else if (item->type() == RiuSelectionItem::GEOMECH_SELECTION_OBJECT)
            {
                const RiuGeoMechSelectionItem* geomechItem = static_cast<const RiuGeoMechSelectionItem*>(item);

                geomechItem->m_view->geoMechCase()->geoMechData()->femParts()->part(geomechItem->m_gridIndex)->structGrid()->ijkFromCellIndex(geomechItem->m_cellIndex, &i, &j, &k);
                gridIndex = geomechItem->m_gridIndex;
                caseId = geomechItem->m_view->geoMechCase()->caseId;
            }
            else
            {
                continue;
            }

            if (caseId == reservoirCase->caseId)
            {
                caseNumber.push_back(static_cast<int>(caseId));
                gridNumber.push_back(static_cast<int>(gridIndex));
                cellI.push_back(static_cast<int>(i + 1));
                cellJ.push_back(static_cast<int>(j + 1));
                cellK.push_back(static_cast<int>(k + 1));
            }
        }
    }
};

static bool RiaGetSelectedCells_init = RiaSocketCommandFactory::instance()->registerCreator<RiaGetSelectedCells>(RiaGetSelectedCells::commandName());
