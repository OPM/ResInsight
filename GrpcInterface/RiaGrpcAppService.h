#pragma once

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

#include "App.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <vector>

namespace rips
{
class Empty;
class Version;
} // namespace rips

namespace caf
{
class PdmValueField;
}

class RiaGrpcCallbackInterface;

class RiaGrpcAppService : public rips::App::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetVersion( grpc::ServerContext* context, const rips::Empty* request, rips::Version* reply ) override;
    grpc::Status Exit( grpc::ServerContext* context, const rips::Empty* request, rips::Empty* reply ) override;
    grpc::Status GetRuntimeInfo( grpc::ServerContext* context, const rips::Empty* request, rips::RuntimeInfo* reply ) override;
    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;
    grpc::Status GetPdmObject( grpc::ServerContext* context, const rips::Empty* request, rips::PdmObject* reply ) override;
};
