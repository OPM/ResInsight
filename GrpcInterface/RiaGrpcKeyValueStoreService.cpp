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
#include "RiaGrpcHelper.h"

// #include "RigActiveCellInfo.h"
// #include "RigActiveCellsResultAccessor.h"
// #include "RigAllGridCellsResultAccessor.h"
// #include "RigCaseCellResultsData.h"
// #include "RigEclipseCaseData.h"
// #include "RigEclipseResultAddress.h"
// #include "RigEclipseResultInfo.h"
// #include "RigMainGrid.h"
// #include "RigResultAccessor.h"
// #include "RigResultAccessorFactory.h"
// #include "RigResultModifier.h"
// #include "RigResultModifierFactory.h"

#include <algorithm>

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
        , m_clientStreamer( clientStreamer )
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
    Status init( const std::string& name )
    {
        m_name = name;
        printf( "Setting name: %s\n", name.c_str() );

        return grpc::Status::OK;

        // int caseId    = request->case_request().id();
        // m_eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( caseId ) );

        // if ( m_eclipseCase )
        // {
        //     m_porosityModel   = static_cast<RiaDefines::PorosityModelType>( request->porosity_model() );
        //     auto   caseData   = m_eclipseCase->eclipseCaseData();
        //     auto   resultData = caseData->results( m_porosityModel );
        //     auto   resultType = static_cast<RiaDefines::ResultCatType>( request->property_type() );
        //     size_t timeStep   = static_cast<size_t>( request->time_step() );

        //     m_resultAddress = RigEclipseResultAddress( resultType, QString::fromStdString( request->property_name() ) );

        //     if ( resultData->ensureKnownResultLoaded( m_resultAddress ) )
        //     {
        //         if ( timeStep < resultData->timeStepCount( m_resultAddress ) )
        //         {
        //             initResultAccess( caseData, request->grid_index(), m_porosityModel, timeStep, m_resultAddress );
        //             return grpc::Status::OK;
        //         }
        //         return grpc::Status( grpc::NOT_FOUND, "No such time step" );
        //     }
        //     else if ( m_clientStreamer )
        //     {
        //         resultData->createResultEntry( m_resultAddress, true );
        //         RigEclipseResultAddress addrToMaxTimeStepCountResult;

        //         size_t timeStepCount = std::max( (size_t)1, resultData->maxTimeStepCount(
        //         &addrToMaxTimeStepCountResult ) );

        //         const std::vector<RigEclipseTimeStepInfo> timeStepInfos =
        //             resultData->timeStepInfos( addrToMaxTimeStepCountResult );
        //         resultData->setTimeStepInfos( m_resultAddress, timeStepInfos );
        //         auto scalarResultFrames = resultData->modifiableCellScalarResultTimesteps( m_resultAddress );
        //         scalarResultFrames->resize( timeStepCount );
        //         if ( timeStep < resultData->timeStepCount( m_resultAddress ) )
        //         {
        //             initResultAccess( caseData, request->grid_index(), m_porosityModel, timeStep, m_resultAddress );

        //             return grpc::Status::OK;
        //         }
        //         return grpc::Status( grpc::NOT_FOUND, "No such time step" );
        //     }
        //     return grpc::Status( grpc::NOT_FOUND, "No such result" );
        // }
        // return grpc::Status( grpc::NOT_FOUND, "Couldn't find an Eclipse case matching the case Id" );
    }

    //--------------------------------------------------------------------------------------------------
    /// Client streamers need to be initialised with the encapsulated parameters
    //--------------------------------------------------------------------------------------------------
    Status init( const KeyValueStoreInputChunk* chunk )
    {
        if ( chunk->has_name() )
        {
            return init( chunk->name() );
        }
        return grpc::Status( grpc::INVALID_ARGUMENT, "Need to have name parameter in first message" );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status receiveStreamRequest( const KeyValueStoreInputChunk* request, ClientToServerStreamReply* reply )
    {
        if ( request->has_values() )
        {
            auto values = request->values().values();
            if ( !values.empty() )
            {
                printf( "Getting values: %d\n", values.size() );

                //     size_t currentCellIdx = m_streamedValueCount;
                //     m_streamedValueCount += values.size();

                //     for ( int i = 0; i < values.size() && currentCellIdx < m_cellCount; ++i, ++currentCellIdx )
                //     {
                //         setCellResult( currentCellIdx, values[i] );
                //     }

                //     if ( m_streamedValueCount > m_cellCount )
                //     {
                //         return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
                //     }

                if ( !values.empty() )
                {
                    for ( int i = 0; i < std::min( 10, static_cast<int>( values.size() ) ); i++ )
                    {
                        printf( "Values: %f\n", values[i] );
                    }

                    m_data.insert( m_data.end(), values.begin(), values.end() );
                }

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
        if ( m_name.empty() && m_data.empty() )
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
    bool               m_clientStreamer;
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
    typedef RiaGrpcKeyValueStoreService Self;

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
