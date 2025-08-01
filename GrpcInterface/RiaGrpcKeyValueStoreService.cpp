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
#include "RiaGrpcKeyValueStoreService.h"

#include "KeyValueStore.pb.h"
#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaGrpcCaseService.h"
#include "RiaGrpcHelper.h"
#include "RiaKeyValueStoreUtil.h"
#include "RiaLogging.h"

#include "cafAssert.h"

using namespace rips;

#define NUM_CONCURRENT_CLIENT_TO_SERVER_STREAMS 10

class RiaKeyValueStoreStateHandler
{
    using Status = grpc::Status;

public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    RiaKeyValueStoreStateHandler( bool clientStreamer = false )
        : m_streamedValueCount( 0u )
        , m_cellCount( 0u )
    {
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    size_t totalValueCount() const { return m_cellCount; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    size_t streamedValueCount() const { return m_streamedValueCount; }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status init( const std::string& name, size_t numElements )
    {
        RiaLogging::debug(
            QString( "Initializing stream: %1 size: %2" ).arg( QString::fromStdString( name ) ).arg( numElements ) );

        m_name      = name;
        m_cellCount = numElements;
        return grpc::Status::OK;
    }

    //--------------------------------------------------------------------------------------------------
    /// Client streamers need to be initialised with the encapsulated parameters
    //--------------------------------------------------------------------------------------------------
    Status init( const KeyValueStoreInputChunk* chunk )
    {
        CAF_ASSERT( chunk );

        if ( chunk->has_parameters() )
        {
            return init( chunk->parameters().name(), chunk->parameters().num_elements() );
        }
        return grpc::Status( grpc::INVALID_ARGUMENT, "Need to have parameters in first message" );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status init( const KeyValueStoreOutputRequest* request )
    {
        std::string name = request->name();

        RiaLogging::debug( QString( "Output stream request. Name='%1'" ).arg( QString::fromStdString( name ) ) );

        auto result = RiaApplication::instance()->keyValueStore()->get( name );
        if ( result.has_value() )
        {
            m_data      = RiaKeyValueStoreUtil::convertToFloatVector( result );
            m_name      = name;
            m_cellCount = m_data.size();
            return grpc::Status::OK;
        }
        else
        {
            return grpc::Status( grpc::NOT_FOUND, "No matching key found." );
        }
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status assignStreamReply( KeyValueStoreOutputChunk* reply )
    {
        // How many data units will fit into one stream package?
        const size_t packageSize   = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( float ) );
        const size_t remainingData = m_cellCount - m_streamedValueCount;
        const size_t chunkSize     = std::min( packageSize, remainingData );

        if ( chunkSize == 0 )
        {
            return grpc::Status( grpc::OUT_OF_RANGE,
                                 "We've reached the end. This is not an error but means transmission is finished" );
        }

        reply->mutable_values()->Reserve( static_cast<int>( chunkSize ) );
        for ( size_t i = 0; i < chunkSize; ++i )
        {
            reply->add_values( m_data[m_streamedValueCount + i] );
        }

        m_streamedValueCount += chunkSize;
        return grpc::Status::OK;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status receiveStreamRequest( const KeyValueStoreInputChunk* request, ClientToServerStreamReply* reply )
    {
        CAF_ASSERT( request );
        CAF_ASSERT( reply );

        if ( request->has_values() )
        {
            auto values = request->values().values();
            if ( !values.empty() )
            {
                size_t currentCellIdx = m_streamedValueCount;
                m_streamedValueCount += values.size();

                if ( !values.empty() )
                {
                    m_data.insert( m_data.end(), values.begin(), values.end() );
                }

                if ( m_streamedValueCount > m_cellCount )
                {
                    return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
                }

                RiaLogging::debug( QString( "Received stream request. Start index: %1. Size after request: %2" )
                                       .arg( currentCellIdx )
                                       .arg( m_data.size() ) );
                reply->set_accepted_value_count( static_cast<int64_t>( m_data.size() ) );
                return Status::OK;
            }
        }
        return Status::OK;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void finish()
    {
        RiaLogging::debug(
            QString( "Stream finished: name=%1 size=%2" ).arg( QString::fromStdString( m_name ) ).arg( m_data.size() ) );
        if ( !m_name.empty() && !m_data.empty() )
        {
            RiaApplication::instance()->keyValueStore()->set( m_name, RiaKeyValueStoreUtil::convertToByteVector( m_data ) );
        }
    }

protected:
    size_t             m_streamedValueCount;
    size_t             m_cellCount;
    std::string        m_name;
    std::vector<float> m_data;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcKeyValueStoreService::SetValue( grpc::ServerContext*                 context,
                                                    const rips::KeyValueStoreInputChunk* request,
                                                    rips::ClientToServerStreamReply*     reply,
                                                    RiaKeyValueStoreStateHandler*        stateHandler )
{
    return stateHandler->receiveStreamRequest( request, reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcKeyValueStoreService::GetValue( grpc::ServerContext*                    context,
                                                    const rips::KeyValueStoreOutputRequest* request,
                                                    rips::KeyValueStoreOutputChunk*         reply,
                                                    RiaKeyValueStoreStateHandler*           stateHandler )
{
    return stateHandler->assignStreamReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcKeyValueStoreService::RemoveValue( grpc::ServerContext*                    context,
                                                       const rips::KeyValueStoreRemoveRequest* request,
                                                       rips::Empty*                            reply )
{
    std::string name = request->name();

    if ( !RiaApplication::instance()->keyValueStore()->exists( name ) )
        return grpc::Status( grpc::NOT_FOUND, "No matching key found." );

    RiaApplication::instance()->keyValueStore()->remove( name );
    return Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcKeyValueStoreService::createCallbacks()
{
    using Self = RiaGrpcKeyValueStoreService;

    std::vector<RiaGrpcCallbackInterface*> callbacks;
    callbacks = { new RiaGrpcClientToServerStreamCallback<Self,
                                                          KeyValueStoreInputChunk,
                                                          ClientToServerStreamReply,
                                                          RiaKeyValueStoreStateHandler>( this,
                                                                                         &Self::SetValue,
                                                                                         &Self::RequestSetValue,
                                                                                         new RiaKeyValueStoreStateHandler(
                                                                                             true ) ),
                  new RiaGrpcUnaryCallback<Self, rips::KeyValueStoreRemoveRequest, rips::Empty>( this,
                                                                                                 &Self::RemoveValue,
                                                                                                 &Self::RequestRemoveValue ) };

    for ( int i = 0; i < NUM_CONCURRENT_CLIENT_TO_SERVER_STREAMS; ++i )
    {
        callbacks.push_back(
            new RiaGrpcServerToClientStreamCallback<Self,
                                                    KeyValueStoreOutputRequest,
                                                    KeyValueStoreOutputChunk,
                                                    RiaKeyValueStoreStateHandler>( this,
                                                                                   &Self::GetValue,
                                                                                   &Self::RequestGetValue,
                                                                                   new RiaKeyValueStoreStateHandler ) );
    }
    return callbacks;
}

static bool RiaGrpcKeyValueStoreService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcKeyValueStoreService>(
        typeid( RiaGrpcKeyValueStoreService ).hash_code() );
