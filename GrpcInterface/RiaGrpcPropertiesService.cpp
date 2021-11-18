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
#include "RiaGrpcPropertiesService.h"

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcCaseService.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigAllGridCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigResultModifier.h"
#include "RigResultModifierFactory.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"

#include "Riu3dSelectionManager.h"

#include <algorithm>

using namespace rips;

#define NUM_CONCURRENT_CLIENT_TO_SERVER_STREAMS 10

//--------------------------------------------------------------------------------------------------
/// Abstract handler base class for streaming cell results to client
///
//--------------------------------------------------------------------------------------------------

class RiaCellResultsStateHandler
{
    typedef grpc::Status Status;

public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    RiaCellResultsStateHandler( bool clientStreamer = false )
        : m_eclipseCase( nullptr )
        , m_porosityModel( RiaDefines::PorosityModelType::MATRIX_MODEL )
        , m_streamedValueCount( 0u )
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
    Status init( const PropertyRequest* request )
    {
        int caseId    = request->case_request().id();
        m_eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcServiceInterface::findCase( caseId ) );

        if ( m_eclipseCase )
        {
            m_porosityModel   = static_cast<RiaDefines::PorosityModelType>( request->porosity_model() );
            auto   caseData   = m_eclipseCase->eclipseCaseData();
            auto   resultData = caseData->results( m_porosityModel );
            auto   resultType = static_cast<RiaDefines::ResultCatType>( request->property_type() );
            size_t timeStep   = static_cast<size_t>( request->time_step() );

            m_resultAddress = RigEclipseResultAddress( resultType, QString::fromStdString( request->property_name() ) );

            if ( resultData->ensureKnownResultLoaded( m_resultAddress ) )
            {
                if ( timeStep < resultData->timeStepCount( m_resultAddress ) )
                {
                    initResultAccess( caseData, request->grid_index(), m_porosityModel, timeStep, m_resultAddress );
                    return grpc::Status::OK;
                }
                return grpc::Status( grpc::NOT_FOUND, "No such time step" );
            }
            else if ( m_clientStreamer )
            {
                resultData->createResultEntry( m_resultAddress, true );
                RigEclipseResultAddress addrToMaxTimeStepCountResult;

                size_t timeStepCount = std::max( (size_t)1, resultData->maxTimeStepCount( &addrToMaxTimeStepCountResult ) );

                const std::vector<RigEclipseTimeStepInfo> timeStepInfos =
                    resultData->timeStepInfos( addrToMaxTimeStepCountResult );
                resultData->setTimeStepInfos( m_resultAddress, timeStepInfos );
                auto scalarResultFrames = resultData->modifiableCellScalarResultTimesteps( m_resultAddress );
                scalarResultFrames->resize( timeStepCount );
                if ( timeStep < resultData->timeStepCount( m_resultAddress ) )
                {
                    initResultAccess( caseData, request->grid_index(), m_porosityModel, timeStep, m_resultAddress );

                    return grpc::Status::OK;
                }
                return grpc::Status( grpc::NOT_FOUND, "No such time step" );
            }
            return grpc::Status( grpc::NOT_FOUND, "No such result" );
        }
        return grpc::Status( grpc::NOT_FOUND, "Couldn't find an Eclipse case matching the case Id" );
    }

    //--------------------------------------------------------------------------------------------------
    /// Client streamers need to be initialised with the encapsulated parameters
    //--------------------------------------------------------------------------------------------------
    Status init( const PropertyInputChunk* chunk )
    {
        if ( chunk->has_params() )
        {
            return init( &( chunk->params() ) );
        }
        return grpc::Status( grpc::INVALID_ARGUMENT, "Need to have PropertyRequest parameters in first message" );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status assignStreamReply( PropertyChunk* reply )
    {
        // How many data units will fit into one stream package?
        const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( sizeof( double ) );
        size_t       indexInPackage = 0u;
        reply->mutable_values()->Reserve( (int)packageSize );

        // Stream until you've reached the package size or total cell count. Whatever comes first.
        // If you've reached the package size you'll come back for another round.
        for ( ; indexInPackage < packageSize && m_streamedValueCount < m_cellCount; ++indexInPackage, ++m_streamedValueCount )
        {
            reply->add_values( cellResult( m_streamedValueCount ) );
        }
        if ( indexInPackage > 0u )
        {
            return grpc::Status::OK;
        }
        return grpc::Status( grpc::OUT_OF_RANGE,
                             "We've reached the end. This is not an error but means transmission is finished" );
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status receiveStreamRequest( const PropertyInputChunk* request, ClientToServerStreamReply* reply )
    {
        if ( request->has_values() )
        {
            auto values = request->values().values();
            if ( !values.empty() )
            {
                size_t currentCellIdx = m_streamedValueCount;
                m_streamedValueCount += values.size();

                for ( int i = 0; i < values.size() && currentCellIdx < m_cellCount; ++i, ++currentCellIdx )
                {
                    setCellResult( currentCellIdx, values[i] );
                }

                if ( m_streamedValueCount > m_cellCount )
                {
                    return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
                }
                reply->set_accepted_value_count( static_cast<int64_t>( currentCellIdx ) );
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
        if ( m_eclipseCase )
        {
            auto caseData   = m_eclipseCase->eclipseCaseData();
            auto resultData = caseData->results( m_porosityModel );
            resultData->recalculateStatistics( m_resultAddress );

            for ( Rim3dView* view : m_eclipseCase->views() )
            {
                view->setCurrentTimeStepAndUpdate( view->currentTimeStep() );
                view->createDisplayModelAndRedraw();
            }
        }
    }

protected:
    virtual void   initResultAccess( RigEclipseCaseData*           caseData,
                                     size_t                        gridIndex,
                                     RiaDefines::PorosityModelType porosityModel,
                                     size_t                        timeStepIndex,
                                     RigEclipseResultAddress       resVarAddr ) = 0;
    virtual double cellResult( size_t currentCellIndex ) const            = 0;
    virtual void   setCellResult( size_t currentCellIndex, double value ) = 0;

protected:
    RimEclipseCase*               m_eclipseCase;
    RiaDefines::PorosityModelType m_porosityModel;
    size_t                        m_streamedValueCount;
    size_t                        m_cellCount;
    bool                          m_clientStreamer;
    RigEclipseResultAddress       m_resultAddress;
};

class RiaActiveCellResultsStateHandler : public RiaCellResultsStateHandler
{
public:
    RiaActiveCellResultsStateHandler( bool clientStreamer = false )
        : RiaCellResultsStateHandler( clientStreamer )
        , m_resultValues( nullptr )
    {
    }

protected:
    void initResultAccess( RigEclipseCaseData*           caseData,
                           size_t                        gridIndex,
                           RiaDefines::PorosityModelType porosityModel,
                           size_t                        timeStepIndex,
                           RigEclipseResultAddress       resVarAddr ) override
    {
        auto activeCellInfo = caseData->activeCellInfo( porosityModel );
        m_resultValues = caseData->results( porosityModel )->modifiableCellScalarResult( resVarAddr, timeStepIndex );
        if ( m_resultValues->empty() )
        {
            m_resultValues->resize( activeCellInfo->reservoirCellResultCount() );
        }
        m_cellCount = activeCellInfo->reservoirActiveCellCount();
    }

    double cellResult( size_t currentCellIndex ) const override { return ( *m_resultValues )[currentCellIndex]; }

    void setCellResult( size_t currentCellIndex, double value ) override
    {
        ( *m_resultValues )[currentCellIndex] = value;
    }

private:
    std::vector<double>* m_resultValues;
};

class RiaSelectedCellResultsStateHandler : public RiaCellResultsStateHandler
{
public:
    RiaSelectedCellResultsStateHandler( bool clientStreamer = false )
        : RiaCellResultsStateHandler( clientStreamer )
    {
    }

protected:
    void initResultAccess( RigEclipseCaseData*           caseData,
                           size_t                        gridIndex,
                           RiaDefines::PorosityModelType porosityModel,
                           size_t                        timeStepIndex,
                           RigEclipseResultAddress       resVarAddr ) override
    {
        std::vector<RiuSelectionItem*> items;
        Riu3dSelectionManager::instance()->selectedItems( items );

        // Only eclipse cases are currently supported. Also filter by case.
        std::vector<RiuEclipseSelectionItem*> eclipseItems;
        for ( auto item : items )
        {
            RiuEclipseSelectionItem* eclipseItem = dynamic_cast<RiuEclipseSelectionItem*>( item );
            if ( eclipseItem && eclipseItem->m_resultDefinition->eclipseCase()->caseId == caseData->ownerCase()->caseId )
            {
                eclipseItems.push_back( eclipseItem );
            }
        }

        m_cellCount = eclipseItems.size();
        if ( m_resultValues.empty() )
        {
            m_resultValues.resize( m_cellCount );
        }

        for ( size_t idx = 0; idx < m_cellCount; idx++ )
        {
            const RiuEclipseSelectionItem* item = eclipseItems[idx];

            CVF_ASSERT( item->type() == RiuSelectionItem::ECLIPSE_SELECTION_OBJECT );
            size_t cellIndex = item->m_gridLocalCellIndex;

            cvf::ref<RigResultAccessor> resultAccessor =
                RigResultAccessorFactory::createFromResultAddress( caseData, gridIndex, porosityModel, timeStepIndex, resVarAddr );

            if ( resultAccessor.isNull() )
            {
                continue;
            }

            double cellValue = resultAccessor->cellScalar( cellIndex );
            if ( cellValue == HUGE_VAL )
            {
                cellValue = 0.0;
            }
            m_resultValues[idx] = cellValue;
        }
    }

    double cellResult( size_t currentCellIndex ) const override { return m_resultValues[currentCellIndex]; }

    void setCellResult( size_t currentCellIndex, double value ) override { m_resultValues[currentCellIndex] = value; }

private:
    std::vector<double> m_resultValues;
};

class RiaGridCellResultsStateHandler : public RiaCellResultsStateHandler
{
public:
    RiaGridCellResultsStateHandler( bool clientStreamer = false )
        : RiaCellResultsStateHandler( clientStreamer )
    {
    }

protected:
    void initResultAccess( RigEclipseCaseData*           caseData,
                           size_t                        gridIndex,
                           RiaDefines::PorosityModelType porosityModel,
                           size_t                        timeStepIndex,
                           RigEclipseResultAddress       resVarAddr ) override
    {
        m_cellCount       = caseData->grid( gridIndex )->cellCount();
        auto resultValues = caseData->results( porosityModel )->modifiableCellScalarResult( resVarAddr, timeStepIndex );
        if ( resultValues && resultValues->empty() && m_cellCount > 0 )
        {
            auto totalCellCount = caseData->mainGrid()->globalCellArray().size();
            resultValues->resize( totalCellCount );
        }

        m_resultAccessor =
            RigResultAccessorFactory::createFromResultAddress( caseData, gridIndex, porosityModel, timeStepIndex, resVarAddr );
        m_resultModifier =
            RigResultModifierFactory::createResultModifier( caseData, gridIndex, porosityModel, timeStepIndex, resVarAddr );
    }

    double cellResult( size_t currentCellIndex ) const override
    {
        return m_resultAccessor->cellScalar( currentCellIndex );
    }

    void setCellResult( size_t currentCellIndex, double value ) override
    {
        return m_resultModifier->setCellScalar( currentCellIndex, value );
    }

private:
    cvf::ref<RigResultAccessor> m_resultAccessor;
    cvf::ref<RigResultModifier> m_resultModifier;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetAvailableProperties( grpc::ServerContext*              context,
                                                               const AvailablePropertiesRequest* request,
                                                               AvailableProperties*              reply )
{
    int             caseId      = request->case_request().id();
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcServiceInterface::findCase( caseId ) );
    if ( eclipseCase )
    {
        auto        porosityModel = static_cast<RiaDefines::PorosityModelType>( request->porosity_model() );
        auto        resultData    = eclipseCase->eclipseCaseData()->results( porosityModel );
        auto        resultType    = static_cast<RiaDefines::ResultCatType>( request->property_type() );
        QStringList resultNames   = resultData->resultNames( resultType );
        if ( !resultNames.empty() )
        {
            for ( QString resultName : resultNames )
            {
                reply->add_property_names( resultName.toStdString() );
            }
            return grpc::Status::OK;
        }
        return grpc::Status( grpc::NOT_FOUND, "Could not find any results matching result type" );
    }
    return grpc::Status( grpc::NOT_FOUND, "No such case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetActiveCellProperty( grpc::ServerContext*              context,
                                                              const PropertyRequest*            request,
                                                              PropertyChunk*                    reply,
                                                              RiaActiveCellResultsStateHandler* stateHandler )
{
    return stateHandler->assignStreamReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetSelectedCellProperty( grpc::ServerContext*                context,
                                                                const PropertyRequest*              request,
                                                                PropertyChunk*                      reply,
                                                                RiaSelectedCellResultsStateHandler* stateHandler )
{
    return stateHandler->assignStreamReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetGridProperty( grpc::ServerContext*            context,
                                                        const rips::PropertyRequest*    request,
                                                        rips::PropertyChunk*            reply,
                                                        RiaGridCellResultsStateHandler* stateHandler )
{
    return stateHandler->assignStreamReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::SetActiveCellProperty( grpc::ServerContext*              context,
                                                              const rips::PropertyInputChunk*   request,
                                                              rips::ClientToServerStreamReply*  reply,
                                                              RiaActiveCellResultsStateHandler* stateHandler )
{
    return stateHandler->receiveStreamRequest( request, reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::SetGridProperty( grpc::ServerContext*             context,
                                                        const rips::PropertyInputChunk*  request,
                                                        rips::ClientToServerStreamReply* reply,
                                                        RiaGridCellResultsStateHandler*  stateHandler )
{
    return stateHandler->receiveStreamRequest( request, reply );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcPropertiesService::createCallbacks()
{
    typedef RiaGrpcPropertiesService Self;

    std::vector<RiaGrpcCallbackInterface*> callbacks;
    callbacks =
        { new RiaGrpcUnaryCallback<Self, AvailablePropertiesRequest, AvailableProperties>( this,
                                                                                           &Self::GetAvailableProperties,
                                                                                           &Self::RequestGetAvailableProperties ),
          new RiaGrpcClientToServerStreamCallback<Self,
                                                  PropertyInputChunk,
                                                  ClientToServerStreamReply,
                                                  RiaActiveCellResultsStateHandler>( this,
                                                                                     &Self::SetActiveCellProperty,
                                                                                     &Self::RequestSetActiveCellProperty,
                                                                                     new RiaActiveCellResultsStateHandler(
                                                                                         true ) ),
          new RiaGrpcClientToServerStreamCallback<Self,
                                                  PropertyInputChunk,
                                                  ClientToServerStreamReply,
                                                  RiaGridCellResultsStateHandler>( this,
                                                                                   &Self::SetGridProperty,
                                                                                   &Self::RequestSetGridProperty,
                                                                                   new RiaGridCellResultsStateHandler(
                                                                                       true ) ) };

    for ( int i = 0; i < NUM_CONCURRENT_CLIENT_TO_SERVER_STREAMS; ++i )
    {
        callbacks.push_back(
            new RiaGrpcServerToClientStreamCallback<Self,
                                                    PropertyRequest,
                                                    PropertyChunk,
                                                    RiaActiveCellResultsStateHandler>( this,
                                                                                       &Self::GetActiveCellProperty,
                                                                                       &Self::RequestGetActiveCellProperty,
                                                                                       new RiaActiveCellResultsStateHandler ) );
        callbacks.push_back(
            new RiaGrpcServerToClientStreamCallback<Self,
                                                    PropertyRequest,
                                                    PropertyChunk,
                                                    RiaGridCellResultsStateHandler>( this,
                                                                                     &Self::GetGridProperty,
                                                                                     &Self::RequestGetGridProperty,
                                                                                     new RiaGridCellResultsStateHandler ) );

        callbacks.push_back(
            new RiaGrpcServerToClientStreamCallback<Self,
                                                    PropertyRequest,
                                                    PropertyChunk,
                                                    RiaSelectedCellResultsStateHandler>( this,
                                                                                         &Self::GetSelectedCellProperty,
                                                                                         &Self::RequestGetSelectedCellProperty,
                                                                                         new RiaSelectedCellResultsStateHandler ) );
    }
    return callbacks;
}

static bool RiaGrpcPropertiesService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcPropertiesService>(
    typeid( RiaGrpcPropertiesService ).hash_code() );
