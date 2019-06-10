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

#include "PdmObject.grpc.pb.h"
#include "RiaGrpcServiceInterface.h"


#include <grpcpp/grpcpp.h>
#include <vector>

//==================================================================================================
//
// gRPC-service answering request searching for PdmObjects in property tree
//
//==================================================================================================
class RiaGrpcPdmObjectService final : public rips::PdmObjectService::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetAncestorPdmObject(grpc::ServerContext*                context,
                                      const rips::PdmParentObjectRequest* request,
                                      rips::PdmObject*                    reply) override;
    grpc::Status GetDescendantPdmObjects(grpc::ServerContext*               context,
                                         const rips::PdmChildObjectRequest* request,
                                         rips::PdmObjectArray*              reply);
    grpc::Status GetChildPdmObjects(grpc::ServerContext*               context,
                                    const rips::PdmChildObjectRequest* request,
                                    rips::PdmObjectArray*              reply);

    grpc::Status
        UpdateExistingPdmObject(grpc::ServerContext* context, const rips::PdmObject* request, rips::Empty* response) override;

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
