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
#include "RiaGrpcGridService.h"

#include "RiaGrpcCallbacks.h"

#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcGridService::GetDimensions( grpc::ServerContext* context, const GridRequest* request, GridDimensions* reply )
{
    RimCase* rimCase = findCase( request->case_request().id() );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( eclipseCase )
    {
        size_t gridIndex = (size_t)request->grid_index();
        if ( gridIndex < eclipseCase->mainGrid()->gridCount() )
        {
            const RigGridBase* grid       = eclipseCase->mainGrid()->gridByIndex( gridIndex );
            Vec3i*             dimensions = new Vec3i;
            dimensions->set_i( (int)grid->cellCountI() );
            dimensions->set_j( (int)grid->cellCountJ() );
            dimensions->set_k( (int)grid->cellCountK() );

            reply->set_allocated_dimensions( dimensions );
            return grpc::Status::OK;
        }
        return grpc::Status( grpc::NOT_FOUND, "Grid not found" );
    }

    return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcGridService::createCallbacks()
{
    typedef RiaGrpcGridService Self;

    return {new RiaGrpcUnaryCallback<Self, GridRequest, GridDimensions>( this,
                                                                         &Self::GetDimensions,
                                                                         &Self::RequestGetDimensions )};
}

static bool RiaGrpcGridService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridService>(
    typeid( RiaGrpcGridService ).hash_code() );
