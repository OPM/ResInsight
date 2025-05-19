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

#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaGrpcCaseService.h"
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
            auto convertFromFloatVectorToBytes = []( const std::vector<float>& float_vec ) -> std::vector<char>
            {
                if ( float_vec.empty() )
                {
                    return {};
                }

                // Calculate the total size needed for the byte array
                size_t size_in_bytes = float_vec.size() * sizeof( float );

                // Create a vector of bytes with the appropriate size
                std::vector<char> byte_vec( size_in_bytes );

                // Copy the binary data from the float vector to the byte vector
                std::memcpy( byte_vec.data(), float_vec.data(), size_in_bytes );

                return byte_vec;
            };

            RiaApplication::instance()->keyValueStore()->set( m_name, convertFromFloatVectorToBytes( m_data ) );
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
std::vector<RiaGrpcCallbackInterface*> RiaGrpcKeyValueStoreService::createCallbacks()
{
    using Self = RiaGrpcKeyValueStoreService;

    std::vector<RiaGrpcCallbackInterface*> callbacks;
    callbacks = {
        new RiaGrpcClientToServerStreamCallback<Self,
                                                KeyValueStoreInputChunk,
                                                ClientToServerStreamReply,
                                                RiaKeyValueStoreStateHandler>( this,
                                                                               &Self::SetValue,
                                                                               &Self::RequestSetValue,
                                                                               new RiaKeyValueStoreStateHandler( true ) ) };
    return callbacks;
}

static bool RiaGrpcKeyValueStoreService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcKeyValueStoreService>(
        typeid( RiaGrpcKeyValueStoreService ).hash_code() );
