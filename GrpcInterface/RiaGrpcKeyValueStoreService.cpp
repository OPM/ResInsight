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

#include <cstring>
#include <string>
#include <type_traits>
#include <utility>

using namespace rips;

#define NUM_CONCURRENT_CLIENT_TO_SERVER_STREAMS 10

// The element types of the store have the same values as the element types on the wire
static_assert( static_cast<int>( RiaKeyValueStoreElementType::FLOAT32 ) == KEY_VALUE_ELEMENT_TYPE_FLOAT32 &&
               static_cast<int>( RiaKeyValueStoreElementType::FLOAT64 ) == KEY_VALUE_ELEMENT_TYPE_FLOAT64 &&
               static_cast<int>( RiaKeyValueStoreElementType::INT32 ) == KEY_VALUE_ELEMENT_TYPE_INT32 &&
               static_cast<int>( RiaKeyValueStoreElementType::INT64 ) == KEY_VALUE_ELEMENT_TYPE_INT64 &&
               static_cast<int>( RiaKeyValueStoreElementType::UINT32 ) == KEY_VALUE_ELEMENT_TYPE_UINT32 &&
               static_cast<int>( RiaKeyValueStoreElementType::UINT64 ) == KEY_VALUE_ELEMENT_TYPE_UINT64 );

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static size_t elementSize( RiaKeyValueStoreElementType elementType )
{
    switch ( elementType )
    {
        case RiaKeyValueStoreElementType::FLOAT32:
        case RiaKeyValueStoreElementType::INT32:
        case RiaKeyValueStoreElementType::UINT32:
            return 4u;
        default:
            return 8u;
    }
}

//--------------------------------------------------------------------------------------------------
/// Name of the element type without the enum prefix, for example UINT32
//--------------------------------------------------------------------------------------------------
static std::string elementTypeName( KeyValueElementType elementType )
{
    const std::string prefix = "KEY_VALUE_ELEMENT_TYPE_";
    return KeyValueElementType_Name( elementType ).substr( prefix.size() );
}

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
        , m_elementType( RiaKeyValueStoreElementType::FLOAT32 )
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
    Status init( const std::string& name, size_t numElements, RiaKeyValueStoreElementType elementType )
    {
        RiaLogging::debug(
            QString( "Initializing stream: %1 size: %2" ).arg( QString::fromStdString( name ) ).arg( numElements ).toStdString() );

        m_name        = name;
        m_cellCount   = numElements;
        m_elementType = elementType;
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
            if ( !KeyValueElementType_IsValid( chunk->parameters().element_type() ) )
            {
                return grpc::Status( grpc::INVALID_ARGUMENT, "Unknown element type" );
            }
            return init( chunk->parameters().name(),
                         chunk->parameters().num_elements(),
                         static_cast<RiaKeyValueStoreElementType>( chunk->parameters().element_type() ) );
        }
        return grpc::Status( grpc::INVALID_ARGUMENT, "Need to have parameters in first message" );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status init( const KeyValueStoreOutputRequest* request )
    {
        std::string name = request->name();

        RiaLogging::debug(
            QString( "Output stream request. Name='%1'" ).arg( QString::fromStdString( name ) ).toStdString() );

        if ( !KeyValueElementType_IsValid( request->element_type() ) )
        {
            return grpc::Status( grpc::INVALID_ARGUMENT, "Unknown element type" );
        }

        auto keyValueStore = RiaApplication::instance()->keyValueStore();
        auto elementType   = static_cast<RiaKeyValueStoreElementType>( request->element_type() );
        auto result        = keyValueStore->get( name, elementType );
        if ( result.has_value() )
        {
            m_data        = result.value();
            m_name        = name;
            m_elementType = elementType;
            m_cellCount   = m_data.size() / elementSize( elementType );
            return grpc::Status::OK;
        }
        else if ( auto storedElementType = keyValueStore->elementType( name ) )
        {
            return grpc::Status( grpc::INVALID_ARGUMENT,
                                 "Key holds values of element type " +
                                     elementTypeName( static_cast<KeyValueElementType>( *storedElementType ) ) +
                                     ", requested " + elementTypeName( request->element_type() ) + "." );
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
        const size_t packageSize   = RiaGrpcHelper::numberOfDataUnitsInPackage( elementSize( m_elementType ) );
        const size_t remainingData = m_cellCount - m_streamedValueCount;
        const size_t chunkSize     = std::min( packageSize, remainingData );

        if ( chunkSize == 0 )
        {
            return grpc::Status( grpc::OUT_OF_RANGE,
                                 "We've reached the end. This is not an error but means transmission is finished" );
        }

        switch ( m_elementType )
        {
            case RiaKeyValueStoreElementType::FLOAT32:
                assignValues<float>( reply->mutable_values(), chunkSize );
                break;
            case RiaKeyValueStoreElementType::FLOAT64:
                assignValues<double>( reply->mutable_double_values(), chunkSize );
                break;
            case RiaKeyValueStoreElementType::INT32:
                assignValues<int32_t>( reply->mutable_int_values(), chunkSize );
                break;
            case RiaKeyValueStoreElementType::INT64:
                assignValues<int64_t>( reply->mutable_int_values(), chunkSize );
                break;
            case RiaKeyValueStoreElementType::UINT32:
                assignValues<uint32_t>( reply->mutable_uint_values(), chunkSize );
                break;
            case RiaKeyValueStoreElementType::UINT64:
                assignValues<uint64_t>( reply->mutable_uint_values(), chunkSize );
                break;
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
            const auto& chunk = request->values();

            if ( Status status = checkValueField( chunk ); !status.ok() ) return status;

            size_t currentCellIdx = m_streamedValueCount;
            Status status         = Status::OK;
            switch ( m_elementType )
            {
                case RiaKeyValueStoreElementType::FLOAT32:
                    status = receiveValues<float>( chunk.values() );
                    break;
                case RiaKeyValueStoreElementType::FLOAT64:
                    status = receiveValues<double>( chunk.double_values() );
                    break;
                case RiaKeyValueStoreElementType::INT32:
                    status = receiveValues<int32_t>( chunk.int_values() );
                    break;
                case RiaKeyValueStoreElementType::INT64:
                    status = receiveValues<int64_t>( chunk.int_values() );
                    break;
                case RiaKeyValueStoreElementType::UINT32:
                    status = receiveValues<uint32_t>( chunk.uint_values() );
                    break;
                case RiaKeyValueStoreElementType::UINT64:
                    status = receiveValues<uint64_t>( chunk.uint_values() );
                    break;
            }
            if ( !status.ok() ) return status;

            if ( m_streamedValueCount > currentCellIdx )
            {
                if ( m_streamedValueCount > m_cellCount )
                {
                    return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
                }

                RiaLogging::debug( QString( "Received stream request. Start index: %1. Size after request: %2" )
                                       .arg( currentCellIdx )
                                       .arg( m_streamedValueCount )
                                       .toStdString() );
                reply->set_accepted_value_count( static_cast<int64_t>( m_streamedValueCount ) );
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
        RiaLogging::debug( QString( "Stream finished: name=%1 size=%2" )
                               .arg( QString::fromStdString( m_name ) )
                               .arg( m_streamedValueCount )
                               .toStdString() );
        // Store only a complete upload, so a stream that failed part way leaves no key behind
        if ( !m_name.empty() && !m_data.empty() && m_streamedValueCount == m_cellCount )
        {
            RiaApplication::instance()->keyValueStore()->set( m_name, m_data, m_elementType );
        }
    }

private:
    //--------------------------------------------------------------------------------------------------
    /// The values of a chunk must be in the field of the element type, otherwise they would be ignored
    //--------------------------------------------------------------------------------------------------
    Status checkValueField( const KeyValueStoreChunk& chunk ) const
    {
        std::string fieldName;
        int         fieldValueCount = 0;
        switch ( m_elementType )
        {
            case RiaKeyValueStoreElementType::FLOAT32:
                fieldName       = "values";
                fieldValueCount = chunk.values_size();
                break;
            case RiaKeyValueStoreElementType::FLOAT64:
                fieldName       = "double_values";
                fieldValueCount = chunk.double_values_size();
                break;
            case RiaKeyValueStoreElementType::INT32:
            case RiaKeyValueStoreElementType::INT64:
                fieldName       = "int_values";
                fieldValueCount = chunk.int_values_size();
                break;
            case RiaKeyValueStoreElementType::UINT32:
            case RiaKeyValueStoreElementType::UINT64:
                fieldName       = "uint_values";
                fieldValueCount = chunk.uint_values_size();
                break;
        }

        const int valueCount = chunk.values_size() + chunk.double_values_size() + chunk.int_values_size() +
                               chunk.uint_values_size();
        if ( valueCount != fieldValueCount )
        {
            return grpc::Status( grpc::INVALID_ARGUMENT,
                                 "Element type " + elementTypeName( static_cast<KeyValueElementType>( m_elementType ) ) +
                                     " expects the values in " + fieldName + "." );
        }
        return Status::OK;
    }

    //--------------------------------------------------------------------------------------------------
    /// Convert the values from the wire type to the stored type, with a range check for integers
    //--------------------------------------------------------------------------------------------------
    template <typename T, typename WireT>
    Status receiveValues( const google::protobuf::RepeatedField<WireT>& values )
    {
        std::vector<T> converted;
        converted.reserve( values.size() );
        for ( WireT value : values )
        {
            if constexpr ( std::is_integral_v<T> )
            {
                if ( !std::in_range<T>( value ) )
                {
                    return grpc::Status( grpc::OUT_OF_RANGE,
                                         "Value " + std::to_string( value ) + " is out of range for element type " +
                                             elementTypeName( static_cast<KeyValueElementType>( m_elementType ) ) + "." );
                }
            }
            converted.push_back( static_cast<T>( value ) );
        }

        auto bytes = RiaKeyValueStoreUtil::convertToByteVector( converted );
        m_data.insert( m_data.end(), bytes.begin(), bytes.end() );
        m_streamedValueCount += converted.size();
        return Status::OK;
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    template <typename T, typename WireT>
    void assignValues( google::protobuf::RepeatedField<WireT>* target, size_t chunkSize )
    {
        target->Reserve( static_cast<int>( chunkSize ) );
        for ( size_t i = 0; i < chunkSize; ++i )
        {
            T value;
            std::memcpy( &value, m_data.data() + ( m_streamedValueCount + i ) * sizeof( T ), sizeof( T ) );
            target->Add( static_cast<WireT>( value ) );
        }
    }

protected:
    size_t                      m_streamedValueCount;
    size_t                      m_cellCount;
    std::string                 m_name;
    RiaKeyValueStoreElementType m_elementType;
    std::vector<char>           m_data;
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
