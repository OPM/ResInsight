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
#pragma once
#include "RiaGrpcServiceInterface.h"

#include "Properties.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <vector>

class RiaActiveCellResultsStateHandler;
class RiaGridCellResultsStateHandler;
class RiaSelectedCellResultsStateHandler;

//==================================================================================================
//
// gRPC-service answering requests about property information for a given case and time step
//
//==================================================================================================
class RiaGrpcPropertiesService final : public rips::Properties::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetAvailableProperties( grpc::ServerContext*                    context,
                                         const rips::AvailablePropertiesRequest* request,
                                         rips::AvailableProperties*              reply ) override;
    grpc::Status GetActiveCellProperty( grpc::ServerContext*              context,
                                        const rips::PropertyRequest*      request,
                                        rips::PropertyChunk*              reply,
                                        RiaActiveCellResultsStateHandler* stateHandler );
    grpc::Status GetSelectedCellProperty( grpc::ServerContext*                context,
                                          const rips::PropertyRequest*        request,
                                          rips::PropertyChunk*                reply,
                                          RiaSelectedCellResultsStateHandler* stateHandler );
    grpc::Status GetGridProperty( grpc::ServerContext*            context,
                                  const rips::PropertyRequest*    request,
                                  rips::PropertyChunk*            reply,
                                  RiaGridCellResultsStateHandler* stateHandler );
    grpc::Status SetActiveCellProperty( grpc::ServerContext*              context,
                                        const rips::PropertyInputChunk*   chunk,
                                        rips::ClientToServerStreamReply*  reply,
                                        RiaActiveCellResultsStateHandler* stateHandler );
    grpc::Status SetGridProperty( grpc::ServerContext*             context,
                                  const rips::PropertyInputChunk*  chunk,
                                  rips::ClientToServerStreamReply* reply,
                                  RiaGridCellResultsStateHandler*  stateHandler );

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
