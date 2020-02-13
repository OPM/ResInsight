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

#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
/// Convert internal ResInsight representation of cells with negative depth to positive depth.
//--------------------------------------------------------------------------------------------------
static inline void convertVec3dToPositiveDepth( cvf::Vec3d* vec )
{
    double& z = vec->z();
    z *= -1;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaCellCenterStateHandler::RiaCellCenterStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_grid( nullptr )
    , m_currentCellIdx( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaCellCenterStateHandler::init( const rips::GridRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    RimCase* rimCase = RiaGrpcServiceInterface::findCase( m_request->case_request().id() );
    m_eclipseCase    = dynamic_cast<RimEclipseCase*>( rimCase );

    if ( !m_eclipseCase )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
    }

    size_t gridIndex = (size_t)request->grid_index();
    if ( gridIndex >= m_eclipseCase->mainGrid()->gridCount() )
    {
        return grpc::Status( grpc::NOT_FOUND, "Grid not found" );
    }

    m_grid = m_eclipseCase->mainGrid()->gridByIndex( gridIndex );

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaCellCenterStateHandler::assignReply( rips::CellCenters* reply )
{
    const size_t packageSize  = RiaGrpcServiceInterface::numberOfMessagesForByteCount( sizeof( rips::CellCenters ) );
    size_t       packageIndex = 0u;
    reply->mutable_centers()->Reserve( (int)packageSize );
    for ( ; packageIndex < packageSize && m_currentCellIdx < m_grid->cellCount(); ++packageIndex )
    {
        cvf::Vec3d center = m_grid->cell( m_currentCellIdx ).center();

        convertVec3dToPositiveDepth( &center );

        Vec3d* cellCenter = reply->add_centers();
        cellCenter->set_x( center.x() );
        cellCenter->set_y( center.y() );
        cellCenter->set_z( center.z() );
        m_currentCellIdx++;
    }
    if ( packageIndex > 0u )
    {
        return Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcGridService::GetDimensions( grpc::ServerContext*     context,
                                                const rips::GridRequest* request,
                                                rips::GridDimensions*    reply )
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
grpc::Status RiaGrpcGridService::GetCellCenters( grpc::ServerContext*       context,
                                                 const rips::GridRequest*   request,
                                                 rips::CellCenters*         reply,
                                                 RiaCellCenterStateHandler* stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcGridService::createCallbacks()
{
    typedef RiaGrpcGridService Self;

    return {

        new RiaGrpcServerToClientStreamCallback<Self, GridRequest, CellCenters, RiaCellCenterStateHandler>( this,
                                                                                                            &Self::GetCellCenters,
                                                                                                            &Self::RequestGetCellCenters,
                                                                                                            new RiaCellCenterStateHandler ),

        new RiaGrpcUnaryCallback<Self, GridRequest, GridDimensions>( this, &Self::GetDimensions, &Self::RequestGetDimensions )};
}

static bool RiaGrpcGridService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridService>( typeid( RiaGrpcGridService ).hash_code() );
