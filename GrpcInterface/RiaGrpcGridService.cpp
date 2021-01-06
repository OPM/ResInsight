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
#include "RiaGrpcHelper.h"

#include "RigCell.h"
#include "RigEclipseCaseData.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"

using namespace rips;

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
    const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( rips::Vec3d ) );
    size_t       indexInPackage = 0u;
    reply->mutable_centers()->Reserve( (int)packageSize );
    for ( ; indexInPackage < packageSize && m_currentCellIdx < m_grid->cellCount(); ++indexInPackage )
    {
        cvf::Vec3d center = m_grid->cell( m_currentCellIdx ).center();

        RiaGrpcHelper::convertVec3dToPositiveDepth( &center );

        Vec3d* cellCenter = reply->add_centers();
        cellCenter->set_x( center.x() );
        cellCenter->set_y( center.y() );
        cellCenter->set_z( center.z() );
        m_currentCellIdx++;
    }
    if ( indexInPackage > 0u )
    {
        return Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaCellCenterStateHandler::assignCornersReply( rips::CellCornersArray* reply )
{
    const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( rips::CellCorners ) );
    size_t       indexInPackage = 0u;
    reply->mutable_cells()->Reserve( (int)packageSize );

    cvf::Vec3d cornerVerts[8];
    for ( ; indexInPackage < packageSize && m_currentCellIdx < m_grid->cellCount(); ++indexInPackage )
    {
        m_grid->cellCornerVertices( m_currentCellIdx, cornerVerts );
        for ( cvf::Vec3d& corner : cornerVerts )
        {
            RiaGrpcHelper::convertVec3dToPositiveDepth( &corner );
        }

        rips::CellCorners* corners = reply->add_cells();
        RiaGrpcHelper::setCornerValues( corners->mutable_c0(), cornerVerts[0] );
        RiaGrpcHelper::setCornerValues( corners->mutable_c1(), cornerVerts[1] );
        RiaGrpcHelper::setCornerValues( corners->mutable_c2(), cornerVerts[2] );
        RiaGrpcHelper::setCornerValues( corners->mutable_c3(), cornerVerts[3] );
        RiaGrpcHelper::setCornerValues( corners->mutable_c4(), cornerVerts[4] );
        RiaGrpcHelper::setCornerValues( corners->mutable_c5(), cornerVerts[5] );
        RiaGrpcHelper::setCornerValues( corners->mutable_c6(), cornerVerts[6] );
        RiaGrpcHelper::setCornerValues( corners->mutable_c7(), cornerVerts[7] );

        m_currentCellIdx++;
    }
    if ( indexInPackage > 0u )
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
grpc::Status RiaGrpcGridService::GetCellCorners( grpc::ServerContext*       context,
                                                 const rips::GridRequest*   request,
                                                 rips::CellCornersArray*    reply,
                                                 RiaCellCenterStateHandler* stateHandler )
{
    return stateHandler->assignCornersReply( reply );
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

        new RiaGrpcServerToClientStreamCallback<Self, GridRequest, CellCornersArray, RiaCellCenterStateHandler>( this,
                                                                                                                 &Self::GetCellCorners,
                                                                                                                 &Self::RequestGetCellCorners,
                                                                                                                 new RiaCellCenterStateHandler ),

        new RiaGrpcUnaryCallback<Self, GridRequest, GridDimensions>( this, &Self::GetDimensions, &Self::RequestGetDimensions ) };
}

static bool RiaGrpcGridService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcGridService>( typeid( RiaGrpcGridService ).hash_code() );
