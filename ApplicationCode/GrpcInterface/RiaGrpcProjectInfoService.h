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

namespace rips
{
class Empty;
class CaseInfo;
}

class RiaGrpcServerCallMethod;

template<typename ServiceT, typename RequestT, typename ReplyT>
class RiaGrpcServerCallData;

//==================================================================================================
//
// gRPC-service answering requests about project information
//
//==================================================================================================
class RiaGrpcProjectInfoService final : public rips::ProjectInfo::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status CurrentCase(grpc::ServerContext* context, const rips::Empty* request, rips::Case* reply) override;
    grpc::Status CurrentCaseInfo(grpc::ServerContext* context, const rips::Empty* request, rips::CaseInfo* reply) override;
    grpc::Status CaseInfoFromCase(grpc::ServerContext* context, const rips::Case* request, rips::CaseInfo* reply) override;
    grpc::Status SelectedCases(grpc::ServerContext* context, const rips::Empty* request, rips::CaseInfos* reply) override;
    grpc::Status AllCaseGroups(grpc::ServerContext* context, const rips::Empty* request, rips::CaseGroups* reply) override;
    grpc::Status AllCases(grpc::ServerContext* context, const rips::Empty* request, rips::CaseInfos* reply) override;
    grpc::Status CasesInGroup(grpc::ServerContext* context, const rips::CaseGroup* request, rips::CaseInfos* reply) override;

public:
    std::vector<RiaGrpcServerCallMethod*> createCallbacks() override;
};


