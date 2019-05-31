/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2019-     Equinor ASA
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
//////////////////////////////////////////////////////////////////////////////////
#include "RiaGrpcCaseService.h"
#include "RiaGrpcCallbacks.h"
#include "RiaSocketTools.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"

#include <string.h> // memcpy

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaActiveCellInfoStateHandler::RiaActiveCellInfoStateHandler()
    : m_request(nullptr)
    , m_eclipseCase(nullptr)
    , m_activeCellInfo(nullptr)
    , m_currentCellIdx(0u)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellInfoStateHandler::init(const rips::CellInfoRequest* request)
{
    CAF_ASSERT(request);
    m_request = request;

    m_porosityModel  = RiaDefines::PorosityModelType(m_request->porosity_model());
    RimCase* rimCase = RiaGrpcServiceInterface::findCase(m_request->case_request().id());
    m_eclipseCase    = dynamic_cast<RimEclipseCase*>(rimCase);

    if (!m_eclipseCase)
    {
        return grpc::Status(grpc::NOT_FOUND, "Eclipse Case not found");
    }

    if (!m_eclipseCase->eclipseCaseData() || !m_eclipseCase->eclipseCaseData()->mainGrid())
    {
        return grpc::Status(grpc::NOT_FOUND, "Eclipse Case Data not found");
    }

    m_activeCellInfo = m_eclipseCase->eclipseCaseData()->activeCellInfo(m_porosityModel);

    if (!m_activeCellInfo)
    {
        return grpc::Status(grpc::NOT_FOUND, "Active Cell Info not found");
    }

    size_t globalCoarseningBoxCount = 0;

    for (size_t gridIdx = 0; gridIdx < m_eclipseCase->eclipseCaseData()->gridCount(); gridIdx++)
    {
        m_globalCoarseningBoxIndexStart.push_back(globalCoarseningBoxCount);

        RigGridBase* grid = m_eclipseCase->eclipseCaseData()->grid(gridIdx);

        size_t localCoarseningBoxCount = grid->coarseningBoxCount();
        globalCoarseningBoxCount += localCoarseningBoxCount;
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellInfoStateHandler::assignNextActiveCellInfoData(rips::CellInfo* cellInfo)
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();

    while (m_currentCellIdx < reservoirCells.size())
    {
        size_t cellIdxToTry = m_currentCellIdx++;
        if (m_activeCellInfo->isActive(cellIdxToTry))
        {
            assignCellInfoData(cellInfo, reservoirCells, cellIdxToTry);
            return grpc::Status::OK;
        }
    }
    return Status(grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaActiveCellInfoStateHandler::assignCellInfoData(rips::CellInfo*       cellInfo,
                                                             const std::vector<RigCell>& reservoirCells,
                                                             size_t                      cellIdx)
{
    RigGridBase* grid = reservoirCells[cellIdx].hostGrid();
    CVF_ASSERT(grid != nullptr);
    size_t cellIndex = reservoirCells[cellIdx].gridLocalCellIndex();

    size_t i, j, k;
    grid->ijkFromCellIndex(cellIndex, &i, &j, &k);

    size_t       pi, pj, pk;
    RigGridBase* parentGrid = nullptr;

    if (grid->isMainGrid())
    {
        pi         = i;
        pj         = j;
        pk         = k;
        parentGrid = grid;
    }
    else
    {
        size_t parentCellIdx = reservoirCells[cellIdx].parentCellIndex();
        parentGrid           = (static_cast<RigLocalGrid*>(grid))->parentGrid();
        CVF_ASSERT(parentGrid != nullptr);
        parentGrid->ijkFromCellIndex(parentCellIdx, &pi, &pj, &pk);
    }

    cellInfo->set_grid_index((int)grid->gridIndex());
    cellInfo->set_parent_grid_index((int)parentGrid->gridIndex());

    size_t coarseningIdx = reservoirCells[cellIdx].coarseningBoxIndex();
    if (coarseningIdx != cvf::UNDEFINED_SIZE_T)
    {
        size_t globalCoarseningIdx = m_globalCoarseningBoxIndexStart[grid->gridIndex()] + coarseningIdx;
        cellInfo->set_coarsening_box_index((int)globalCoarseningIdx);
    }
    else
    {
        cellInfo->set_coarsening_box_index(-1);
    }
    {
        rips::Vec3i* local_ijk = new rips::Vec3i;
        local_ijk->set_i((int)i);
        local_ijk->set_j((int)j);
        local_ijk->set_k((int)k);
        cellInfo->set_allocated_local_ijk(local_ijk);
    }
    {
        rips::Vec3i* parent_ijk = new rips::Vec3i;
        parent_ijk->set_i((int)pi);
        parent_ijk->set_j((int)pj);
        parent_ijk->set_k((int)pk);
        cellInfo->set_allocated_parent_ijk(parent_ijk);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RigActiveCellInfo* RiaActiveCellInfoStateHandler::activeCellInfo() const
{
    return m_activeCellInfo;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<RigCell>& RiaActiveCellInfoStateHandler::reservoirCells() const
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();
    return reservoirCells;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellInfoStateHandler::assignReply(rips::CellInfoArray* reply)
{
    const size_t packageSize = RiaGrpcServiceInterface::numberOfMessagesForByteCount(sizeof(rips::CellInfoArray));
    size_t       packageIndex = 0u;
    reply->mutable_data()->Reserve((int)packageSize);
    for (; packageIndex < packageSize && m_currentCellIdx < m_activeCellInfo->reservoirCellCount(); ++packageIndex)
    {
        rips::CellInfo  singleCellInfo;
        grpc::Status          singleCellInfoStatus = assignNextActiveCellInfoData(&singleCellInfo);
        if (singleCellInfoStatus.ok())
        {
            rips::CellInfo* allocCellInfo = reply->add_data();
            *allocCellInfo = singleCellInfo;
        }
        else
        {
            break;
        }
    }
    if (packageIndex > 0u)
    {
        return Status::OK;
    }
    return Status(grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetGridCount(grpc::ServerContext* context, const rips::CaseRequest* request, rips::GridCount* reply)
{
    RimCase* rimCase = findCase(request->id());

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    if (eclipseCase)
    {
        int gridCount = (int) eclipseCase->mainGrid()->gridCount();
        reply->set_count(gridCount);
        return Status::OK;
    }
    return grpc::Status(grpc::NOT_FOUND, "Eclipse Case not found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellCount(grpc::ServerContext* context, const rips::CellInfoRequest* request, rips::CellCount* reply)
{
    RimCase* rimCase = findCase(request->case_request().id());

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    if (eclipseCase)
    {
        auto porosityModel = RiaDefines::PorosityModelType(request->porosity_model());
        RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo(porosityModel);
        reply->set_active_cell_count((int) activeCellInfo->reservoirActiveCellCount());
        reply->set_reservoir_cell_count((int) activeCellInfo->reservoirCellCount());
        return grpc::Status::OK;
    }
    return grpc::Status(grpc::NOT_FOUND, "Eclipse Case not found");
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
RiaGrpcCaseService::GetTimeSteps(grpc::ServerContext* context, const rips::CaseRequest* request, rips::TimeStepDates* reply)
{
    RimCase* rimCase = findCase(request->id());

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    if (eclipseCase)
    {
        std::vector<QDateTime> timeStepDates = eclipseCase->timeStepDates();
        for (QDateTime dateTime : timeStepDates)
        {
            rips::TimeStepDate* date = reply->add_dates();
            date->set_year(dateTime.date().year());
            date->set_month(dateTime.date().month());
            date->set_day(dateTime.date().day());
            date->set_hour(dateTime.time().hour());
            date->set_minute(dateTime.time().minute());
            date->set_second(dateTime.time().second());
        }
        return grpc::Status::OK;
    }
    return grpc::Status(grpc::NOT_FOUND, "Eclipse Case not found");
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcCaseService::GetCaseInfo(grpc::ServerContext* context, const rips::CaseRequest* request, rips::CaseInfo* reply)
{
    RimCase* rimCase = findCase(request->id());
    if (rimCase)
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase(rimCase, caseId, caseName, caseType, caseGroupId);

        reply->set_id(caseId);
        reply->set_group_id(caseGroupId);
        reply->set_name(caseName.toStdString());
        reply->set_type(caseType.toStdString());
        return Status::OK;
    }
    return Status(grpc::NOT_FOUND, "No cases found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellInfoForActiveCells(grpc::ServerContext*                      context,
                                                                const rips::CellInfoRequest*        request,
                                                                rips::CellInfoArray*                reply,
                                                                RiaActiveCellInfoStateHandler* stateHandler)
{
    return stateHandler->assignReply(reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcCaseService::createCallbacks()
{
    typedef RiaGrpcCaseService Self;

    return {new RiaGrpcUnaryCallback<Self, CaseRequest, GridCount>(this, &Self::GetGridCount, &Self::RequestGetGridCount),
            new RiaGrpcUnaryCallback<Self, CellInfoRequest, CellCount>(this, &Self::GetCellCount, &Self::RequestGetCellCount),
            new RiaGrpcUnaryCallback<Self, CaseRequest, TimeStepDates>(this, &Self::GetTimeSteps, &Self::RequestGetTimeSteps),
            new RiaGrpcUnaryCallback<Self, CaseRequest, CaseInfo>(this, &Self::GetCaseInfo, &Self::RequestGetCaseInfo),
            new RiaGrpcServerStreamCallback<Self, CellInfoRequest, CellInfoArray, RiaActiveCellInfoStateHandler>(
                this, &Self::GetCellInfoForActiveCells, &Self::RequestGetCellInfoForActiveCells, new RiaActiveCellInfoStateHandler)};
}

static bool RiaGrpcCaseService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcCaseService>(typeid(RiaGrpcCaseService).hash_code());
