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

#include "Commands.grpc.pb.h"

#include <grpcpp/grpcpp.h>
#include <vector>

namespace rips
{
class Empty;
}

namespace caf
{
class PdmValueField;
}

namespace google
{
namespace protobuf
{
class FieldDescriptor;
class Message;
}
}

class RiaGrpcServerCallMethod;

class RiaGrpcCommandService : public rips::Commands::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status Execute(grpc::ServerContext* context, const rips::CommandParams* request, rips::Empty* reply) override;
    std::vector<RiaGrpcServerCallMethod*> createCallbacks() override;
private:
    void assignFieldValue(caf::PdmValueField* pdmValueField, const google::protobuf::Message& params, const google::protobuf::FieldDescriptor* paramDescriptor);
};
