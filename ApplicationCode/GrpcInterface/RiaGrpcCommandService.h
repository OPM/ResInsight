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

namespace caf
{
class PdmChildFieldHandle;
class PdmValueField;
class PdmObject;
class PdmObjectHandle;
template <typename T>
class PdmField;
} // namespace caf

namespace google
{
namespace protobuf
{
    class FieldDescriptor;
    class Message;
} // namespace protobuf
} // namespace google

class RiaGrpcCallbackInterface;

class RiaGrpcCommandService : public rips::Commands::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status
                                           Execute( grpc::ServerContext* context, const rips::CommandParams* request, rips::CommandReply* reply ) override;
    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;

private:
    template <typename T>
    static caf::PdmField<T>* dataValueField( caf::PdmValueField* valueField );
    template <typename T>
    static const caf::PdmField<T>* constDataValueField( const caf::PdmValueField* valueField );
    void                           assignPdmFieldValue( caf::PdmValueField*                      pdmValueField,
                                                        const google::protobuf::Message&         params,
                                                        const google::protobuf::FieldDescriptor* paramDescriptor );
    void                           assignPdmObjectValues( caf::PdmObjectHandle*                    pdmObject,
                                                          const google::protobuf::Message&         params,
                                                          const google::protobuf::FieldDescriptor* paramDescriptor );

    void assignGrpcFieldValue( google::protobuf::Message*               reply,
                               const google::protobuf::FieldDescriptor* fieldDescriptor,
                               const caf::PdmValueField*                pdmValueField );
    void assignResultToReply( const caf::PdmObject* result, rips::CommandReply* reply );
};
