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
#include "RiaGrpcHelper.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigAllGridCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigFemPart.h"
#include "RigFemPartCollection.h"
#include "RigFemPartResultsCollection.h"
#include "RigFemResultAddress.h"
#include "RigGeoMechCaseData.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigResultModifier.h"
#include "RigResultModifierFactory.h"

#include "Rim3dView.h"
#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"
#include "RimGeoMechCase.h"

#include "Riu3dSelectionManager.h"

#include <algorithm>
#include <expected>
#include <optional>

using namespace rips;

namespace
{
//--------------------------------------------------------------------------------------------------
/// Geomechanical results are addressed by a GEOMECH_* property type (the position) and "FIELD:COMPONENT"
//--------------------------------------------------------------------------------------------------
std::optional<RigFemResultPosEnum> geoMechResultPosition( rips::PropertyType propertyType )
{
    switch ( propertyType )
    {
        case rips::PropertyType::GEOMECH_NODAL:
            return RIG_NODAL;
        case rips::PropertyType::GEOMECH_ELEMENT_NODAL:
            return RIG_ELEMENT_NODAL;
        case rips::PropertyType::GEOMECH_INTEGRATION_POINT:
            return RIG_INTEGRATION_POINT;
        case rips::PropertyType::GEOMECH_ELEMENT:
            return RIG_ELEMENT;
        default:
            return std::nullopt;
    }
}

//--------------------------------------------------------------------------------------------------
/// One value per element, parts after each other: the mean over the element's nodes for nodal and
/// element-nodal results.
//--------------------------------------------------------------------------------------------------
std::expected<std::vector<double>, std::string> geoMechElementValues( RimGeoMechCase*        geoMechCase,
                                                                      const PropertyRequest* request )
{
    auto position = geoMechResultPosition( request->property_type() );
    if ( !position ) return std::unexpected( "Use a GEOMECH_* property type for a geomechanical case" );

    RigGeoMechCaseData* caseData = geoMechCase->geoMechData();
    if ( !caseData ) return std::unexpected( "Geomechanical case data not found" );

    const QString             name = QString::fromStdString( request->property_name() );
    const RigFemResultAddress address( *position,
                                       name.section( ':', 0, 0 ).toStdString(),
                                       name.section( ':', 1, 1 ).toStdString() );

    RigFemPartResultsCollection* results = caseData->femPartResults();
    if ( request->time_step() < 0 || request->time_step() >= results->totalSteps() )
        return std::unexpected( "No such time step" );
    const auto [stepIndex, frameIndex] = results->stepListIndexToTimeStepAndDataFrameIndex( request->time_step() );

    std::vector<double>         elementValues;
    const RigFemPartCollection* parts = caseData->femParts();
    for ( int partIndex = 0; partIndex < parts->partCount(); partIndex++ )
    {
        const RigFemPart*         part   = parts->part( partIndex );
        const std::vector<float>& values = results->resultValues( address, partIndex, stepIndex, frameIndex );
        if ( values.empty() ) return std::unexpected( "No such result" );

        for ( int elementIdx = 0; elementIdx < part->elementCount(); elementIdx++ )
        {
            const int nodeCount = RigFemTypes::elementNodeCount( part->elementType( elementIdx ) );
            double    sum       = 0.0;
            for ( int localNode = 0; localNode < nodeCount; localNode++ )
            {
                sum += values[part->resultValueIdxFromResultPosType( *position, elementIdx, localNode )];
            }
            elementValues.push_back( sum / nodeCount );
        }
    }
    return elementValues;
}
} // namespace

#define NUM_CONCURRENT_CLIENT_TO_SERVER_STREAMS 10

//--------------------------------------------------------------------------------------------------
/// Abstract handler base class for streaming cell results to client
///
//--------------------------------------------------------------------------------------------------

class RiaCellResultsStateHandler
{
    using Status = grpc::Status;

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
        m_eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( caseId ) );

        if ( m_eclipseCase )
        {
            // This function will import grid geometry and result data. For this operation, only access to cell result
            // data is required.
            m_eclipseCase->ensureReservoirCaseIsOpen();
            auto caseData = m_eclipseCase->eclipseCaseData();
            if ( !caseData )
            {
                return grpc::Status( grpc::NOT_FOUND, "Eclipse case data not found" );
            }

            m_porosityModel   = static_cast<RiaDefines::PorosityModelType>( request->porosity_model() );
            auto   resultData = caseData->results( m_porosityModel );
            auto   resultType = static_cast<RiaDefines::ResultCatType>( request->property_type() );
            size_t timeStep   = static_cast<size_t>( request->time_step() );

            m_resultAddress = RigEclipseResultAddress( resultType, QString::fromStdString( request->property_name() ) );

            auto dataType = ( request->data_type() == rips::PropertyDataType::INTEGER )
                                ? RiaDefines::ResultDataType::INTEGER
                                : RiaDefines::ResultDataType::FLOAT;
            m_resultAddress.setDataType( dataType );

            if ( resultData->ensureKnownResultLoaded( m_resultAddress ) )
            {
                if ( timeStep < resultData->timeStepCount( m_resultAddress ) )
                {
                    // Equality on RigEclipseResultAddress ignores dataType, so an existing entry
                    // keeps its original dataType. Patch it when overwriting from the client.
                    if ( m_clientStreamer ) resultData->updateResultDataType( m_resultAddress, dataType );
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
        if ( auto geoMechCase = dynamic_cast<RimGeoMechCase*>( RiaGrpcHelper::findCase( caseId ) ) )
        {
            if ( m_clientStreamer || !supportsGeoMech() )
            {
                return grpc::Status( grpc::UNIMPLEMENTED, "Geomechanical results can only be read for all elements" );
            }
            auto values = geoMechElementValues( geoMechCase, request );
            if ( !values ) return grpc::Status( grpc::NOT_FOUND, values.error() );

            m_geoMechValues = std::move( *values );
            m_cellCount     = m_geoMechValues.size();
            return grpc::Status::OK;
        }
        return grpc::Status( grpc::NOT_FOUND, "Couldn't find an Eclipse case matching the case Id" );
    }

    //--------------------------------------------------------------------------------------------------
    /// Client streamers need to be initialized with the encapsulated parameters
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
        const size_t packageSize    = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( double ) );
        size_t       indexInPackage = 0u;
        reply->mutable_values()->Reserve( (int)packageSize );

        // Stream until you've reached the package size or total cell count. Whatever comes first.
        // If you've reached the package size you'll come back for another round.
        for ( ; indexInPackage < packageSize && m_streamedValueCount < m_cellCount; ++indexInPackage, ++m_streamedValueCount )
        {
            reply->add_values( m_geoMechValues.empty() ? cellResult( m_streamedValueCount )
                                                       : m_geoMechValues[m_streamedValueCount] );
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
    // Only reading all elements is implemented for geomechanical cases
    virtual bool supportsGeoMech() const { return false; }

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
    std::vector<double>           m_geoMechValues;
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
    bool supportsGeoMech() const override { return true; }

    void initResultAccess( RigEclipseCaseData*           caseData,
                           size_t                        gridIndex,
                           RiaDefines::PorosityModelType porosityModel,
                           size_t                        timeStepIndex,
                           RigEclipseResultAddress       resVarAddr ) override
    {
        auto activeCellInfo = caseData->activeCellInfo( porosityModel );
        m_cellCount         = activeCellInfo->reservoirActiveCellCount();
        m_resultValues = caseData->results( porosityModel )->modifiableCellScalarResult( resVarAddr, timeStepIndex );
        if ( m_resultValues->empty() )
        {
            m_resultValues->resize( activeCellInfo->reservoirActiveCellCount() );
        }
        else if ( m_resultValues->size() != m_cellCount )
        {
            // Filter out inactive cells
            std::vector<double> activeCellResultValues( m_cellCount );
            size_t              reservoirCellCount = activeCellInfo->reservoirCellCount();
            for ( size_t cellIdx = 0; cellIdx < reservoirCellCount; cellIdx++ )
            {
                size_t activeCellIdx =
                    caseData->activeCellInfo( m_porosityModel )->cellResultIndex( ReservoirCellIndex( cellIdx ) ).value();
                if ( activeCellIdx != cvf::UNDEFINED_SIZE_T )
                    activeCellResultValues[activeCellIdx] = ( *m_resultValues )[cellIdx];
            }

            // Use the filtered result
            m_resultValues->resize( m_cellCount );
            for ( size_t activeIdx = 0; activeIdx < m_cellCount; activeIdx++ )
            {
                ( *m_resultValues )[activeIdx] = activeCellResultValues[activeIdx];
            }
        }
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
            if ( eclipseItem && eclipseItem->m_resultDefinition->eclipseCase()->caseId() == caseData->ownerCase()->caseId() )
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
            auto totalCellCount = caseData->mainGrid()->totalCellCount();
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
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( caseId ) );
    if ( eclipseCase )
    {
        // This function will import grid geometry and result data. For this operation, only access to cell result
        // data is required.
        eclipseCase->ensureReservoirCaseIsOpen();
        auto caseData = eclipseCase->eclipseCaseData();
        if ( !caseData )
        {
            return grpc::Status( grpc::NOT_FOUND, "Eclipse case data not found" );
        }

        auto        porosityModel = static_cast<RiaDefines::PorosityModelType>( request->porosity_model() );
        auto        resultData    = caseData->results( porosityModel );
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
    if ( auto geoMechCase = dynamic_cast<RimGeoMechCase*>( RiaGrpcHelper::findCase( caseId ) ) )
    {
        auto position = geoMechResultPosition( request->property_type() );
        if ( !position )
            return grpc::Status( grpc::INVALID_ARGUMENT, "Use a GEOMECH_* property type for a geomechanical case" );
        if ( !geoMechCase->geoMechData() ) return grpc::Status( grpc::NOT_FOUND, "Geomechanical case data not found" );

        for ( const auto& [field, components] :
              geoMechCase->geoMechData()->femPartResults()->scalarFieldAndComponentNames( *position ) )
        {
            if ( components.empty() ) reply->add_property_names( field );
            for ( const auto& component : components )
            {
                reply->add_property_names( field + ":" + component );
            }
        }
        return grpc::Status::OK;
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
    using Self = RiaGrpcPropertiesService;

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
