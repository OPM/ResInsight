/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RiaGrpcNNCPropertiesService.h"

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcCaseService.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RimEclipseCase.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaNNCConnectionsStateHandler::RiaNNCConnectionsStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_currentIdx( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCConnectionsStateHandler::init( const rips::CaseRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    RimCase* rimCase = RiaGrpcServiceInterface::findCase( m_request->id() );
    m_eclipseCase    = dynamic_cast<RimEclipseCase*>( rimCase );

    if ( !( m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->mainGrid() ) )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
rips::Vec3i* createConnectionVec3i( const RigCell& cell )
{
    RigGridBase* hostGrid           = cell.hostGrid();
    size_t       gridLocalCellIndex = cell.gridLocalCellIndex();
    size_t       i, j, k;
    hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k );

    rips::Vec3i* vec = new rips::Vec3i;
    vec->set_i( i );
    vec->set_j( j );
    vec->set_k( k );

    return vec;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCConnectionsStateHandler::assignReply( rips::NNCConnections* reply )
{
    RigMainGrid*               mainGrid    = m_eclipseCase->eclipseCaseData()->mainGrid();
    std::vector<RigConnection> connections = mainGrid->nncData()->connections();

    size_t       connectionCount = connections.size();
    const size_t packageSize     = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( rips::NNCConnection ) );
    size_t       indexInPackage  = 0u;
    reply->mutable_connections()->Reserve( (int)packageSize );
    for ( ; indexInPackage < packageSize && m_currentIdx < connectionCount; ++indexInPackage )
    {
        RigConnection& connection = connections[m_currentIdx];
        const RigCell& cell1      = mainGrid->globalCellArray()[connection.m_c1GlobIdx];
        const RigCell& cell2      = mainGrid->globalCellArray()[connection.m_c2GlobIdx];

        NNCConnection* nncConnection = reply->add_connections();
        nncConnection->set_allocated_cell1( createConnectionVec3i( cell1 ) );
        nncConnection->set_allocated_cell2( createConnectionVec3i( cell2 ) );
        nncConnection->set_cell_grid_index1( cell1.hostGrid()->gridIndex() );
        nncConnection->set_cell_grid_index2( cell2.hostGrid()->gridIndex() );

        m_currentIdx++;
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
grpc::Status RiaGrpcNNCPropertiesService::GetNNCConnections( grpc::ServerContext*           context,
                                                             const rips::CaseRequest*       request,
                                                             rips::NNCConnections*          reply,
                                                             RiaNNCConnectionsStateHandler* stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcNNCPropertiesService::GetAvailableNNCProperties( grpc::ServerContext*    context,
                                                                     const CaseRequest*      request,
                                                                     AvailableNNCProperties* reply )
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcServiceInterface::findCase( request->id() ) );
    if ( eclipseCase && eclipseCase->eclipseCaseData() && eclipseCase->eclipseCaseData()->mainGrid() )
    {
        RigNNCData* nncData = eclipseCase->eclipseCaseData()->mainGrid()->nncData();

        std::vector<RigNNCData::NNCResultType> resultTypes;
        resultTypes.push_back( RigNNCData::NNC_DYNAMIC );
        resultTypes.push_back( RigNNCData::NNC_STATIC );
        resultTypes.push_back( RigNNCData::NNC_GENERATED );

        for ( size_t rtIdx = 0; rtIdx < resultTypes.size(); ++rtIdx )
        {
            std::vector<QString> availableParameters = nncData->availableProperties( resultTypes[rtIdx] );

            for ( const QString& parameter : availableParameters )
            {
                AvailableNNCProperty* property = reply->add_properties();
                property->set_name( parameter.toStdString() );
                property->set_property_type( static_cast<NNCPropertyType>( resultTypes[rtIdx] ) );
            }
        }

        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "No such case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcNNCPropertiesService::createCallbacks()
{
    typedef RiaGrpcNNCPropertiesService Self;

    std::vector<RiaGrpcCallbackInterface*> callbacks;
    callbacks = {new RiaGrpcUnaryCallback<Self, CaseRequest, AvailableNNCProperties>( this,
                                                                                      &Self::GetAvailableNNCProperties,
                                                                                      &Self::RequestGetAvailableNNCProperties ),
                 new RiaGrpcServerToClientStreamCallback<Self,
                                                         CaseRequest,
                                                         rips::NNCConnections,
                                                         RiaNNCConnectionsStateHandler>( this,
                                                                                         &Self::GetNNCConnections,
                                                                                         &Self::RequestGetNNCConnections,
                                                                                         new RiaNNCConnectionsStateHandler )};
    return callbacks;
}

static bool RiaGrpcNNCPropertiesService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcNNCPropertiesService>(
        typeid( RiaGrpcNNCPropertiesService ).hash_code() );
