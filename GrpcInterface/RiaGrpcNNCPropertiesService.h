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
#pragma once
#include "RiaGrpcServiceInterface.h"

#include "NNCProperties.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <vector>

#include <QString>

class RimEclipseCase;

//==================================================================================================
//
// State handler for streaming of NNC connections
//
//==================================================================================================
class RiaNNCConnectionsStateHandler
{
    typedef grpc::Status Status;

public:
    RiaNNCConnectionsStateHandler();
    grpc::Status init( const rips::CaseRequest* request );
    grpc::Status assignReply( rips::NNCConnections* reply );

protected:
    const rips::CaseRequest* m_request;
    RimEclipseCase*          m_eclipseCase;
    size_t                   m_currentIdx;
};

//==================================================================================================
//
// State handler for streaming of NNC values
//
//==================================================================================================
class RiaNNCValuesStateHandler
{
    typedef grpc::Status Status;

public:
    RiaNNCValuesStateHandler();
    grpc::Status init( const rips::NNCValuesRequest* request );
    grpc::Status assignReply( rips::NNCValues* reply );

protected:
    const rips::NNCValuesRequest* m_request;
    RimEclipseCase*               m_eclipseCase;
    size_t                        m_currentIdx;
};

//==================================================================================================
//
// State handler for client-to-server streaming of NNC values
//
//==================================================================================================

class RiaNNCInputValuesStateHandler
{
public:
    typedef grpc::Status Status;

public:
    RiaNNCInputValuesStateHandler( bool t = true );
    grpc::Status init( const rips::NNCValuesChunk* request );
    grpc::Status init( const rips::NNCValuesInputRequest* request );
    grpc::Status receiveStreamRequest( const rips::NNCValuesChunk* request, rips::ClientToServerStreamReply* reply );

    size_t totalValueCount() const;
    size_t streamedValueCount() const;
    void   finish();

protected:
    RimEclipseCase* m_eclipseCase;
    size_t          m_cellCount;
    size_t          m_streamedValueCount;
    size_t          m_timeStep;
    QString         m_propertyName;
};

//==================================================================================================
//
// gRPC-service answering requests about NNC property information for a given case and time step
//
//==================================================================================================
class RiaGrpcNNCPropertiesService final : public rips::NNCProperties::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetAvailableNNCProperties( grpc::ServerContext*          context,
                                            const rips::CaseRequest*      request,
                                            rips::AvailableNNCProperties* reply ) override;
    grpc::Status GetNNCConnections( grpc::ServerContext*           context,
                                    const rips::CaseRequest*       request,
                                    rips::NNCConnections*          reply,
                                    RiaNNCConnectionsStateHandler* stateHandler );
    grpc::Status GetNNCValues( grpc::ServerContext*          context,
                               const rips::NNCValuesRequest* request,
                               rips::NNCValues*              reply,
                               RiaNNCValuesStateHandler*     stateHandler );
    grpc::Status SetNNCValues( grpc::ServerContext*             context,
                               const rips::NNCValuesChunk*      chunk,
                               rips::ClientToServerStreamReply* reply,
                               RiaNNCInputValuesStateHandler*   stateHandler );

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
