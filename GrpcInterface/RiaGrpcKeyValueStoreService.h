/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025-     Equinor ASA
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

#include "KeyValueStore.grpc.pb.h"

#include <grpcpp/grpcpp.h>

class RiaKeyValueStoreStateHandler;

//==================================================================================================
//
// gRPC-service answering requests about property information for a given case and time step
//
//==================================================================================================
class RiaGrpcKeyValueStoreService final : public rips::KeyValueStore::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status SetValue( grpc::ServerContext*                 context,
                           const rips::KeyValueStoreInputChunk* chunk,
                           rips::ClientToServerStreamReply*     reply,
                           RiaKeyValueStoreStateHandler*        stateHandler );

    grpc::Status GetValue( grpc::ServerContext*                    context,
                           const rips::KeyValueStoreOutputRequest* request,
                           rips::KeyValueStoreOutputChunk*         reply,
                           RiaKeyValueStoreStateHandler*           stateHandler );

    grpc::Status RemoveValue( grpc::ServerContext*                    context,
                              const rips::KeyValueStoreRemoveRequest* request,
                              rips::Empty*                            reply ) override;

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
