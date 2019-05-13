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
#include "RiaGrpcGridInfoService.h"
#include "RiaGrpcCallbacks.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"

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
grpc::Status RiaActiveCellInfoStateHandler::init(const rips::ActiveCellInfoRequest* request)
{
    CAF_ASSERT(request);
    m_request = request;

    m_porosityModel  = RiaDefines::PorosityModelType(m_request->porosity_model());
    RimCase* rimCase = RiaGrpcServiceInterface::findCase(m_request->case_id());
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
grpc::Status RiaActiveCellInfoStateHandler::assignNextActiveCellInfoData(rips::ActiveCellInfo* cellInfo)
{
    const std::vector<RigCell>& reservoirCells = m_eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();

    while (m_currentCellIdx < reservoirCells.size())
    {
        size_t cellIdxToTry = m_currentCellIdx++;
        if (m_activeCellInfo->isActive(cellIdxToTry))
        {
            assignActiveCellInfoData(cellInfo, reservoirCells, cellIdxToTry);
            return grpc::Status::OK;
        }
    }
    return Status(grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaActiveCellInfoStateHandler::assignActiveCellInfoData(rips::ActiveCellInfo*       cellInfo,
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
RiaActiveCellInfoStreamStateHandler::RiaActiveCellInfoStreamStateHandler()
    : RiaActiveCellInfoStateHandler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellInfoStreamStateHandler::assignReply(rips::ActiveCellInfo* reply)
{
    return assignNextActiveCellInfoData(reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaActiveCellInfoChunkStreamStateHandler::RiaActiveCellInfoChunkStreamStateHandler()
    : RiaActiveCellInfoStateHandler()
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellInfoChunkStreamStateHandler::assignReply(rips::ActiveCellInfoArray* reply)
{
    const size_t packageSize  = 1024u;
    size_t       packageIndex = 0u;
    reply->mutable_data()->Reserve(packageSize);
    for (; packageIndex < packageSize && m_currentCellIdx < m_activeCellInfo->reservoirCellCount(); ++packageIndex)
    {
        rips::ActiveCellInfo* singleCellInfo       = reply->add_data();
        grpc::Status          singleCellInfoStatus = assignNextActiveCellInfoData(singleCellInfo);
        if (singleCellInfoStatus.error_code() == grpc::OUT_OF_RANGE)
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
grpc::Status RiaGrpcGridInfoService::GetGridCount(grpc::ServerContext* context, const rips::Case* request, rips::GridCount* reply)
{
    RimCase* rimCase = findCase(request->id());

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    size_t          gridCount   = 0u;
    if (eclipseCase)
    {
        gridCount = eclipseCase->mainGrid()->gridCount();
        reply->set_count((int)gridCount);
        return Status::OK;
    }
    return grpc::Status(grpc::NOT_FOUND, "Eclipse Case not found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridInfoService::GetAllGridDimensions(grpc::ServerContext*     context,
                                                          const rips::Case*        request,
                                                          rips::AllGridDimensions* reply)
{
    RimCase* rimCase = findCase(request->id());

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    if (eclipseCase)
    {
        size_t gridCount = eclipseCase->mainGrid()->gridCount();
        for (size_t i = 0; i < gridCount; ++i)
        {
            const RigGridBase* grid       = eclipseCase->mainGrid()->gridByIndex(i);
            rips::Vec3i*       dimensions = reply->add_grid_dimensions();
            dimensions->set_i((int)grid->cellCountI());
            dimensions->set_j((int)grid->cellCountJ());
            dimensions->set_k((int)grid->cellCountK());
        }
        return grpc::Status::OK;
    }

    return grpc::Status(grpc::NOT_FOUND, "Eclipse Case not found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridInfoService::GetActiveCellInfoArray(grpc::ServerContext*               context,
                                                            const rips::ActiveCellInfoRequest* request,
                                                            rips::ActiveCellInfoArray*         reply)
{
    RiaActiveCellInfoChunkStreamStateHandler stateHandler;
    grpc::Status                             initStatus = stateHandler.init(request);
    if (!initStatus.ok())
    {
        return initStatus;
    }

    RigActiveCellInfo* activeCellInfo = stateHandler.activeCellInfo();
    CAF_ASSERT(activeCellInfo);

    reply->mutable_data()->Reserve((int)activeCellInfo->reservoirActiveCellCount());

    const std::vector<RigCell>& reservoirCells     = stateHandler.reservoirCells();
    size_t                      reservoirCellCount = reservoirCells.size();
    for (size_t cIdx = 0; cIdx < reservoirCellCount; ++cIdx)
    {
        if (stateHandler.activeCellInfo()->isActive(cIdx))
        {
            rips::ActiveCellInfo* cellInfo = reply->add_data();
            stateHandler.assignActiveCellInfoData(cellInfo, reservoirCells, cIdx);
        }
    }
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridInfoService::StreamActiveCellInfo(grpc::ServerContext*                 context,
                                                          const rips::ActiveCellInfoRequest*   request,
                                                          rips::ActiveCellInfo*                reply,
                                                          RiaActiveCellInfoStreamStateHandler* stateHandler)
{
    return stateHandler->assignReply(reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridInfoService::StreamActiveCellInfoChunks(grpc::ServerContext*                      context,
                                                                const rips::ActiveCellInfoRequest*        request,
                                                                rips::ActiveCellInfoArray*                reply,
                                                                RiaActiveCellInfoChunkStreamStateHandler* stateHandler)
{
    return stateHandler->assignReply(reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaAbstractGrpcCallback*> RiaGrpcGridInfoService::createCallbacks()
{
    typedef RiaGrpcGridInfoService Self;

    return {
        new RiaGrpcCallback<Self, Case, GridCount>(this, &Self::GetGridCount, &Self::RequestGetGridCount),
        new RiaGrpcCallback<Self, Case, AllGridDimensions>(this, &Self::GetAllGridDimensions, &Self::RequestGetAllGridDimensions),
        new RiaGrpcCallback<Self, ActiveCellInfoRequest, ActiveCellInfoArray>(
            this, &Self::GetActiveCellInfoArray, &Self::RequestGetActiveCellInfoArray),
        new RiaGrpcStreamCallback<Self, ActiveCellInfoRequest, rips::ActiveCellInfo, RiaActiveCellInfoStreamStateHandler>(
            this, &Self::StreamActiveCellInfo, &Self::RequestStreamActiveCellInfo, new RiaActiveCellInfoStreamStateHandler),
        new RiaGrpcStreamCallback<Self, ActiveCellInfoRequest, ActiveCellInfoArray, RiaActiveCellInfoChunkStreamStateHandler>(
            this,
            &Self::StreamActiveCellInfoChunks,
            &Self::RequestStreamActiveCellInfoChunks,
            new RiaActiveCellInfoChunkStreamStateHandler)};
}

static bool RiaGrpcGridInfoService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridInfoService>(typeid(RiaGrpcGridInfoService).hash_code());
