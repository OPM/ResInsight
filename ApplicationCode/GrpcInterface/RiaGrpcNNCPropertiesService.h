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

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
