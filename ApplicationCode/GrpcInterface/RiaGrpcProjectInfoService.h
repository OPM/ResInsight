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
#include "ProjectInfo.grpc.pb.h"

#include <grpcpp/grpcpp.h>

namespace ResInsight
{
class Empty;
class CaseInfo;
}

//==================================================================================================
//
// gRPC-service answering requests about project information
//
//==================================================================================================
class RiaGrpcProjectInfoService final : public ResInsight::ProjectInfo::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status CurrentCase(grpc::ServerContext* context, const ResInsight::Empty* request, ResInsight::CaseInfo* reply) override;
    grpc::Status SelectedCases(grpc::ServerContext* context, const ResInsight::Empty* request, ResInsight::CaseInfos* reply) override;
    grpc::Status AllCaseGroups(grpc::ServerContext* context, const ResInsight::Empty* request, ResInsight::CaseGroups* reply) override;
    grpc::Status AllCases(grpc::ServerContext* context, const ResInsight::Empty* request, ResInsight::CaseInfos* reply) override;
    grpc::Status CasesInGroup(grpc::ServerContext* context, const ResInsight::CaseGroup* request, ResInsight::CaseInfos* reply) override;

public:
    std::vector<RiaGrpcServerCallMethod*> createCallbacks(grpc::ServerCompletionQueue* cq) override;
};


