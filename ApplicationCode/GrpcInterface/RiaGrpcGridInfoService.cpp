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
#include "RiaGrpcProjectInfoService.h"
#include "RiaGrpcServerCallData.h"

#include "RigActiveCellInfo.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimGeoMechCase.h"


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridInfoService::GridCount(grpc::ServerContext*             context,
                                               const ResInsight::Case*          request,
                                               ResInsight::GridCountInfo*       reply)
{
    RimCase* rimCase = findCase(request->id());

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    size_t gridCount = 0u;
    if (eclipseCase)
    {
        gridCount = eclipseCase->mainGrid()->gridCount();
    }
    reply->set_count((int)gridCount);
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridInfoService::AllDimensions(grpc::ServerContext*        context,
                                                       const ResInsight::Case*     request,
                                                       ResInsight::AllGridDimensions* reply)
{
    RimCase* rimCase = findCase(request->id());

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);
    if (eclipseCase)
    {
        size_t gridCount = eclipseCase->mainGrid()->gridCount();
        for (size_t i = 0; i < gridCount; ++i)
        {
            const RigGridBase* grid = eclipseCase->mainGrid()->gridByIndex(i);
            ResInsight::Vec3i* dimensions = reply->add_dimensions();
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
grpc::Status RiaGrpcGridInfoService::AllActiveCellInfos(grpc::ServerContext*                     context,
                                                        const ResInsight::ActiveCellInfoRequest* request,
                                                        ResInsight::ActiveCellInfos*             reply)
{
    RimCase* rimCase = findCase(request->case_id());
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(rimCase);

    if (!eclipseCase) return grpc::Status(grpc::NOT_FOUND, "Eclipse Case not found");
    if (!eclipseCase->eclipseCaseData() || !eclipseCase->eclipseCaseData()->mainGrid())
    {
        return grpc::Status(grpc::NOT_FOUND, "Eclipse Case Data not found");
    }
    RiaDefines::PorosityModelType porosityModel = RiaDefines::PorosityModelType(request->porosity_model());

    RigActiveCellInfo* activeCellInfo  = eclipseCase->eclipseCaseData()->activeCellInfo(porosityModel);
    size_t             activeCellCount = activeCellInfo->reservoirActiveCellCount();

    std::vector<size_t> globalCoarseningBoxIndexStart;
    {
        size_t globalCoarseningBoxCount = 0;

        for (size_t gridIdx = 0; gridIdx < eclipseCase->eclipseCaseData()->gridCount(); gridIdx++)
        {
            globalCoarseningBoxIndexStart.push_back(globalCoarseningBoxCount);

            RigGridBase* grid = eclipseCase->eclipseCaseData()->grid(gridIdx);

            size_t localCoarseningBoxCount = grid->coarseningBoxCount();
            globalCoarseningBoxCount += localCoarseningBoxCount;
        }
    }

    reply->mutable_data()->Reserve((int) activeCellCount);
    const std::vector<RigCell>& reservoirCells = eclipseCase->eclipseCaseData()->mainGrid()->globalCellArray();
    for (size_t cIdx = 0; cIdx < reservoirCells.size(); ++cIdx)
    {
        if (activeCellInfo->isActive(cIdx))
        {
            RigGridBase* grid = reservoirCells[cIdx].hostGrid();
            CVF_ASSERT(grid != nullptr);
            size_t cellIndex = reservoirCells[cIdx].gridLocalCellIndex();

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
                size_t parentCellIdx = reservoirCells[cIdx].parentCellIndex();
                parentGrid           = (static_cast<RigLocalGrid*>(grid))->parentGrid();
                CVF_ASSERT(parentGrid != nullptr);
                parentGrid->ijkFromCellIndex(parentCellIdx, &pi, &pj, &pk);
            }
            ResInsight::ActiveCellInfo* cellInfo = reply->add_data();
            
            cellInfo->set_grid_index((int)grid->gridIndex());
            cellInfo->set_parent_grid_index((int)parentGrid->gridIndex());

            size_t coarseningIdx = reservoirCells[cIdx].coarseningBoxIndex();
            if (coarseningIdx != cvf::UNDEFINED_SIZE_T)
            {
                size_t globalCoarseningIdx = globalCoarseningBoxIndexStart[grid->gridIndex()] + coarseningIdx;
                cellInfo->set_coarsening_box_index((int) globalCoarseningIdx);
            }
            else
            {
                cellInfo->set_coarsening_box_index(-1);
            }
            {
                ResInsight::Vec3i* local_ijk = new ResInsight::Vec3i;
                local_ijk->set_i((int)i);
                local_ijk->set_j((int)j);
                local_ijk->set_k((int)k);
                cellInfo->set_allocated_local_ijk(local_ijk);
            }
            {
                ResInsight::Vec3i* parent_ijk = new ResInsight::Vec3i;
                parent_ijk->set_i((int)pi);
                parent_ijk->set_j((int)pj);
                parent_ijk->set_k((int)pk);
                cellInfo->set_allocated_parent_ijk(parent_ijk);
            }
        }
    }
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcServerCallMethod*> RiaGrpcGridInfoService::createCallbacks(grpc::ServerCompletionQueue* cq)
{
    std::vector<RiaGrpcServerCallMethod*> callbacks;
    callbacks.push_back(new RiaGrpcServerCallData<RiaGrpcGridInfoService, ResInsight::Case, ResInsight::GridCountInfo>(
        this, cq, "GridCount", &RiaGrpcGridInfoService::GridCount, &RiaGrpcGridInfoService::RequestGridCount));
    callbacks.push_back(new RiaGrpcServerCallData<RiaGrpcGridInfoService, ResInsight::Case, ResInsight::AllGridDimensions>(
        this, cq, "AllDimensions", &RiaGrpcGridInfoService::AllDimensions, &RiaGrpcGridInfoService::RequestAllDimensions));
    callbacks.push_back(new RiaGrpcServerCallData<RiaGrpcGridInfoService, ResInsight::ActiveCellInfoRequest, ResInsight::ActiveCellInfos>(
        this, cq, "AllActiveCellInfos", &RiaGrpcGridInfoService::AllActiveCellInfos, &RiaGrpcGridInfoService::RequestAllActiveCellInfos));
    return callbacks;
}

