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

namespace caf
{
class PdmProxyFieldHandle;
}

struct AbstractDataHolder
{
    virtual size_t dataCount() const                                                             = 0;
    virtual size_t dataSizeOf() const                                                            = 0;
    virtual void   reserveReplyStorage( rips::PdmObjectGetterReply* reply ) const                = 0;
    virtual void   addValueToReply( size_t valueIndex, rips::PdmObjectGetterReply* reply ) const = 0;

    virtual size_t getValuesFromChunk( size_t startIndex, const rips::PdmObjectSetterChunk* chunk ) = 0;
    virtual void   applyValuesToProxyField( caf::PdmProxyFieldHandle* proxyField )                  = 0;
};

//==================================================================================================
//
// State handler for streaming of active cell info
//
//==================================================================================================
class RiaPdmObjectMethodStateHandler
{
    typedef grpc::Status Status;

public:
    RiaPdmObjectMethodStateHandler( bool clientToServerStreamer = false );

    Status init( const rips::PdmObjectGetterRequest* request );
    Status init( const rips::PdmObjectSetterChunk* chunk );
    Status assignReply( rips::PdmObjectGetterReply* reply );
    Status receiveRequest( const rips::PdmObjectSetterChunk* chunk, rips::ClientToServerStreamReply* reply );
    size_t streamedValueCount() const;
    size_t totalValueCount() const;
    void   finish();

protected:
    caf::PdmObject*                     m_fieldOwner;
    caf::PdmProxyFieldHandle*           m_proxyField;
    std::unique_ptr<AbstractDataHolder> m_dataHolder;
    size_t                              m_currentDataIndex;
    bool                                m_clientToServerStreamer;
};

//==================================================================================================
//
// gRPC-service answering request searching for PdmObjects in property tree
//
//==================================================================================================
class RiaGrpcPdmObjectService final : public rips::PdmObjectService::AsyncService, public RiaGrpcServiceInterface
{
public:
    grpc::Status GetAncestorPdmObject( grpc::ServerContext*                context,
                                       const rips::PdmParentObjectRequest* request,
                                       rips::PdmObject*                    reply ) override;
    grpc::Status GetDescendantPdmObjects( grpc::ServerContext*                    context,
                                          const rips::PdmDescendantObjectRequest* request,
                                          rips::PdmObjectArray*                   reply ) override;
    grpc::Status GetChildPdmObjects( grpc::ServerContext*               context,
                                     const rips::PdmChildObjectRequest* request,
                                     rips::PdmObjectArray*              reply ) override;

    grpc::Status CreateChildPdmObject( grpc::ServerContext*                     context,
                                       const rips::CreatePdmChildObjectRequest* request,
                                       rips::PdmObject*                         reply ) override;

    grpc::Status UpdateExistingPdmObject( grpc::ServerContext*   context,
                                          const rips::PdmObject* request,
                                          rips::Empty*           response ) override;

    grpc::Status CallPdmObjectGetter( grpc::ServerContext*                context,
                                      const rips::PdmObjectGetterRequest* request,
                                      rips::PdmObjectGetterReply*         reply,
                                      RiaPdmObjectMethodStateHandler*     stateHandler );
    grpc::Status CallPdmObjectSetter( grpc::ServerContext*              context,
                                      const rips::PdmObjectSetterChunk* chunk,
                                      rips::ClientToServerStreamReply*  reply,
                                      RiaPdmObjectMethodStateHandler*   stateHandler );
    grpc::Status CallPdmObjectMethod( grpc::ServerContext*                context,
                                      const rips::PdmObjectMethodRequest* request,
                                      rips::PdmObject*                    reply ) override;

    std::vector<RiaGrpcCallbackInterface*> createCallbacks() override;

    static caf::PdmObject* findCafObjectFromRipsObject( const rips::PdmObject& ripsObject );
    static caf::PdmObject* findCafObjectFromScriptNameAndAddress( const QString& scriptClassName, uint64_t address );
};
