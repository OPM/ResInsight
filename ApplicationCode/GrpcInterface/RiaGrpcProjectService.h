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

#include "Project.grpc.pb.h"
#include "RiaGrpcServiceInterface.h"

#include <grpcpp/grpcpp.h>

namespace rips
{
class Empty;
class CaseRequest;
} // namespace rips

class RiaGrpcCallbackInterface;

//==================================================================================================
//
// gRPC-service answering requests about project information
//
//==================================================================================================
class RiaGrpcProjectService final : public rips::Project::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetCurrentCase( grpc::ServerContext* context, const rips::Empty* request, rips::CaseRequest* reply ) override;
    grpc::Status
        GetSelectedCases( grpc::ServerContext* context, const rips::Empty* request, rips::CaseInfoArray* reply ) override;
    grpc::Status
                 GetAllCaseGroups( grpc::ServerContext* context, const rips::Empty* request, rips::CaseGroups* reply ) override;
    grpc::Status GetAllCases( grpc::ServerContext* context, const rips::Empty* request, rips::CaseInfoArray* reply ) override;
    grpc::Status GetCasesInGroup( grpc::ServerContext*   context,
                                  const rips::CaseGroup* request,
                                  rips::CaseInfoArray*   reply ) override;
    grpc::Status GetPdmObject( grpc::ServerContext* context, const rips::Empty* request, rips::PdmObject* reply ) override;

public:
    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
};
