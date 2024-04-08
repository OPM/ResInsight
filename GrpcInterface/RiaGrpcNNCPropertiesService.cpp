/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2020-     Equinor ASA
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
#include "RiaGrpcNNCPropertiesService.h"

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcCaseService.h"
#include "RiaGrpcHelper.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigNNCData.h"

#include "RimEclipseCase.h"
#include "RimEclipseCellColors.h"
#include "RimEclipseInputCase.h"
#include "RimEclipseInputProperty.h"
#include "RimEclipseInputPropertyCollection.h"
#include "RimEclipseView.h"
#include "RimIntersectionCollection.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaNNCConnectionsStateHandler::RiaNNCConnectionsStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_currentIdx( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCConnectionsStateHandler::init( const rips::CaseRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    RimCase* rimCase = RiaGrpcHelper::findCase( m_request->id() );
    m_eclipseCase    = dynamic_cast<RimEclipseCase*>( rimCase );

    if ( !( m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->mainGrid() ) )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
rips::Vec3i* createConnectionVec3i( const RigCell& cell )
{
    RigGridBase* hostGrid           = cell.hostGrid();
    size_t       gridLocalCellIndex = cell.gridLocalCellIndex();
    size_t       i, j, k;
    hostGrid->ijkFromCellIndex( gridLocalCellIndex, &i, &j, &k );

    rips::Vec3i* vec = new rips::Vec3i;
    vec->set_i( i );
    vec->set_j( j );
    vec->set_k( k );

    return vec;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCConnectionsStateHandler::assignReply( rips::NNCConnections* reply )
{
    RigMainGrid*                  mainGrid    = m_eclipseCase->eclipseCaseData()->mainGrid();
    const RigConnectionContainer& connections = mainGrid->nncData()->allConnections();

    size_t       connectionCount = connections.size();
    const size_t packageSize     = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( rips::NNCConnection ) );
    size_t       indexInPackage  = 0u;
    reply->mutable_connections()->Reserve( (int)packageSize );
    for ( ; indexInPackage < packageSize && m_currentIdx < connectionCount; ++indexInPackage )
    {
        const RigConnection& connection = connections[m_currentIdx];
        const RigCell&       cell1      = mainGrid->globalCellArray()[connection.c1GlobIdx()];
        const RigCell&       cell2      = mainGrid->globalCellArray()[connection.c2GlobIdx()];

        NNCConnection* nncConnection = reply->add_connections();
        nncConnection->set_allocated_cell1( createConnectionVec3i( cell1 ) );
        nncConnection->set_allocated_cell2( createConnectionVec3i( cell2 ) );
        nncConnection->set_cell_grid_index1( cell1.hostGrid()->gridIndex() );
        nncConnection->set_cell_grid_index2( cell2.hostGrid()->gridIndex() );

        m_currentIdx++;
    }

    if ( indexInPackage > 0u )
    {
        return Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcNNCPropertiesService::GetNNCConnections( grpc::ServerContext*           context,
                                                             const rips::CaseRequest*       request,
                                                             rips::NNCConnections*          reply,
                                                             RiaNNCConnectionsStateHandler* stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaNNCValuesStateHandler::RiaNNCValuesStateHandler()
    : m_request( nullptr )
    , m_eclipseCase( nullptr )
    , m_currentIdx( 0u )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCValuesStateHandler::init( const rips::NNCValuesRequest* request )
{
    CAF_ASSERT( request );
    m_request = request;

    RimCase* rimCase = RiaGrpcHelper::findCase( m_request->case_id() );
    m_eclipseCase    = dynamic_cast<RimEclipseCase*>( rimCase );

    if ( !( m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->mainGrid() ) )
    {
        return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
    }

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
const std::vector<double>* getScalarResultByName( const RigNNCData*         nncData,
                                                  RigNNCData::NNCResultType resultType,
                                                  const QString&            propertyName,
                                                  size_t                    timeStep )
{
    if ( resultType == RigNNCData::NNCResultType::NNC_STATIC )
    {
        return nncData->staticConnectionScalarResultByName( propertyName );
    }

    if ( resultType == RigNNCData::NNCResultType::NNC_DYNAMIC )
    {
        return nncData->dynamicConnectionScalarResultByName( propertyName, timeStep );
    }

    if ( resultType == RigNNCData::NNCResultType::NNC_GENERATED )
    {
        return nncData->generatedConnectionScalarResultByName( propertyName, timeStep );
    }

    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCValuesStateHandler::assignReply( rips::NNCValues* reply )
{
    RigMainGrid* mainGrid    = m_eclipseCase->eclipseCaseData()->mainGrid();
    auto         connections = mainGrid->nncData()->allConnections();

    QString                   propertyName = QString::fromStdString( m_request->property_name() );
    RigNNCData::NNCResultType propertyType = static_cast<RigNNCData::NNCResultType>( m_request->property_type() );
    size_t                    timeStep     = m_request->time_step();

    const std::vector<double>* nncValues =
        getScalarResultByName( mainGrid->nncData(), propertyType, propertyName, timeStep );
    if ( !nncValues )
    {
        return Status( grpc::NOT_FOUND, "No values found" );
    }

    size_t       connectionCount = connections.size();
    const size_t packageSize     = RiaGrpcHelper::numberOfDataUnitsInPackage( sizeof( double ) );
    size_t       indexInPackage  = 0u;
    reply->mutable_values()->Reserve( (int)packageSize );
    for ( ; indexInPackage < packageSize && m_currentIdx < connectionCount; ++indexInPackage )
    {
        reply->add_values( nncValues->at( m_currentIdx ) );
        m_currentIdx++;
    }

    if ( indexInPackage > 0u )
    {
        return Status::OK;
    }
    return Status( grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcNNCPropertiesService::GetNNCValues( grpc::ServerContext*          context,
                                                        const rips::NNCValuesRequest* request,
                                                        rips::NNCValues*              reply,
                                                        RiaNNCValuesStateHandler*     stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcNNCPropertiesService::GetAvailableNNCProperties( grpc::ServerContext*    context,
                                                                     const CaseRequest*      request,
                                                                     AvailableNNCProperties* reply )
{
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( request->id() ) );
    if ( eclipseCase && eclipseCase->eclipseCaseData() && eclipseCase->eclipseCaseData()->mainGrid() )
    {
        RigNNCData* nncData = eclipseCase->eclipseCaseData()->mainGrid()->nncData();

        std::vector<RigNNCData::NNCResultType> resultTypes;
        resultTypes.push_back( RigNNCData::NNCResultType::NNC_DYNAMIC );
        resultTypes.push_back( RigNNCData::NNCResultType::NNC_STATIC );
        resultTypes.push_back( RigNNCData::NNCResultType::NNC_GENERATED );

        for ( size_t rtIdx = 0; rtIdx < resultTypes.size(); ++rtIdx )
        {
            std::vector<QString> availableParameters = nncData->availableProperties( resultTypes[rtIdx] );

            for ( const QString& parameter : availableParameters )
            {
                AvailableNNCProperty* property = reply->add_properties();
                property->set_name( parameter.toStdString() );
                property->set_property_type( static_cast<NNCPropertyType>( resultTypes[rtIdx] ) );
            }
        }

        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "No such case" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool scalarResultExistsOrCreate( RigCaseCellResultsData* results, QString propertyName )
{
    RigEclipseResultAddress resAddr( RiaDefines::ResultCatType::GENERATED, propertyName );

    if ( !results->ensureKnownResultLoaded( resAddr ) )
    {
        results->createResultEntry( resAddr, true );
    }

    std::vector<std::vector<double>>* scalarResultFrames = results->modifiableCellScalarResultTimesteps( resAddr );
    size_t                            timeStepCount      = results->maxTimeStepCount();
    scalarResultFrames->resize( timeStepCount );

    return true;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
static bool createIJKCellResults( RigCaseCellResultsData* results, QString propertyName )
{
    bool ok;
    ok = scalarResultExistsOrCreate( results, QString( "%1IJK" ).arg( propertyName ) );
    if ( !ok ) return false;
    ok = scalarResultExistsOrCreate( results, QString( "%1I" ).arg( propertyName ) );
    if ( !ok ) return false;
    ok = scalarResultExistsOrCreate( results, QString( "%1J" ).arg( propertyName ) );
    if ( !ok ) return false;
    ok = scalarResultExistsOrCreate( results, QString( "%1K" ).arg( propertyName ) );

    return ok;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaNNCInputValuesStateHandler::RiaNNCInputValuesStateHandler( bool )
    : m_eclipseCase( nullptr )
    , m_streamedValueCount( 0u )
    , m_cellCount( 0u )
    , m_timeStep( 0u )
{
}

std::vector<double>* getOrCreateConnectionScalarResultByName( RigNNCData* nncData, const QString propertyName, int timeStep )
{
    std::vector<double>* resultsToAdd = nncData->generatedConnectionScalarResultByName( propertyName, timeStep );
    if ( resultsToAdd )
    {
        return resultsToAdd;
    }
    else
    {
        nncData->makeGeneratedConnectionScalarResult( propertyName, timeStep + 1 );
        return nncData->generatedConnectionScalarResultByName( propertyName, timeStep );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCInputValuesStateHandler::init( const NNCValuesInputRequest* request )
{
    int caseId    = request->case_id();
    m_eclipseCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( caseId ) );

    if ( m_eclipseCase && m_eclipseCase->eclipseCaseData() && m_eclipseCase->eclipseCaseData()->mainGrid() )
    {
        auto caseData        = m_eclipseCase->eclipseCaseData();
        auto m_porosityModel = static_cast<RiaDefines::PorosityModelType>( request->porosity_model() );
        m_timeStep           = request->time_step();
        m_propertyName       = QString::fromStdString( request->property_name() );

        RigNNCData* nncData = m_eclipseCase->eclipseCaseData()->mainGrid()->nncData();
        std::vector<double>* resultsToAdd = getOrCreateConnectionScalarResultByName( nncData, m_propertyName, m_timeStep );
        if ( !resultsToAdd )
        {
            return grpc::Status( grpc::NOT_FOUND, "No results for scalar results found." );
        }

        if ( !m_eclipseCase->results( m_porosityModel ) )
        {
            return grpc::Status( grpc::NOT_FOUND, "No results for porosity model." );
        }

        bool ok = createIJKCellResults( m_eclipseCase->results( m_porosityModel ), m_propertyName );
        if ( !ok )
        {
            return grpc::Status( grpc::NOT_FOUND, "Could not find the property results." );
        }

        RigEclipseResultAddress resAddr( QString( "%1IJK" ).arg( m_propertyName ) );
        m_eclipseCase->results( m_porosityModel )->ensureKnownResultLoaded( resAddr );
        nncData->setEclResultAddress( m_propertyName, resAddr );

        m_cellCount = caseData->mainGrid()->nncData()->allConnections().size();

        resultsToAdd->resize( m_cellCount, HUGE_VAL );

        return Status::OK;
    }

    return grpc::Status( grpc::NOT_FOUND, "Couldn't find an Eclipse case matching the case Id" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaNNCInputValuesStateHandler::init( const rips::NNCValuesChunk* chunk )
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
grpc::Status RiaNNCInputValuesStateHandler::receiveStreamRequest( const NNCValuesChunk*      request,
                                                                  ClientToServerStreamReply* reply )
{
    if ( request->has_values() )
    {
        auto values = request->values().values();
        if ( !values.empty() )
        {
            RigNNCData* nncData = m_eclipseCase->eclipseCaseData()->mainGrid()->nncData();

            std::vector<std::vector<double>>* resultsToAdd =
                nncData->generatedConnectionScalarResultByName( m_propertyName );

            size_t currentCellIdx = m_streamedValueCount;
            m_streamedValueCount += values.size();

            for ( int i = 0; i < values.size() && currentCellIdx < m_cellCount; ++i, ++currentCellIdx )
            {
                resultsToAdd->at( m_timeStep )[currentCellIdx] = values[i];
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
size_t RiaNNCInputValuesStateHandler::totalValueCount() const
{
    return m_cellCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaNNCInputValuesStateHandler::streamedValueCount() const
{
    return m_streamedValueCount;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaNNCInputValuesStateHandler::finish()
{
    if ( m_eclipseCase != nullptr )
    {
        // Create a new input property if we have an input reservoir
        RimEclipseInputCase* inputRes = dynamic_cast<RimEclipseInputCase*>( m_eclipseCase );
        if ( inputRes )
        {
            RimEclipseInputProperty* inputProperty =
                inputRes->inputPropertyCollection()->findInputProperty( m_propertyName );
            if ( !inputProperty )
            {
                inputProperty                 = new RimEclipseInputProperty;
                inputProperty->resultName     = m_propertyName;
                inputProperty->eclipseKeyword = "";
                inputProperty->fileName       = QString( "" );
                inputRes->inputPropertyCollection()->inputProperties.push_back( inputProperty );
                inputRes->inputPropertyCollection()->updateConnectedEditors();
            }
            inputProperty->resolvedState = RimEclipseInputProperty::RESOLVED_NOT_SAVED;
        }

        for ( RimEclipseView* view : m_eclipseCase->reservoirViews() )
        {
            // As new result might have been introduced, update all editors connected
            view->cellResult()->updateConnectedEditors();

            // It is usually not needed to create new display model, but if any derived geometry based on
            // generated data (from Octave) a full display model rebuild is required
            view->scheduleCreateDisplayModelAndRedraw();
            view->intersectionCollection()->scheduleCreateDisplayModelAndRedraw2dIntersectionViews();
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcNNCPropertiesService::SetNNCValues( grpc::ServerContext*             context,
                                                        const rips::NNCValuesChunk*      chunk,
                                                        rips::ClientToServerStreamReply* reply,
                                                        RiaNNCInputValuesStateHandler*   stateHandler )
{
    return stateHandler->receiveStreamRequest( chunk, reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcNNCPropertiesService::createCallbacks()
{
    typedef RiaGrpcNNCPropertiesService Self;

    std::vector<RiaGrpcCallbackInterface*> callbacks;
    callbacks =
        { new RiaGrpcUnaryCallback<Self, CaseRequest, AvailableNNCProperties>( this,
                                                                               &Self::GetAvailableNNCProperties,
                                                                               &Self::RequestGetAvailableNNCProperties ),
          new RiaGrpcServerToClientStreamCallback<Self,
                                                  CaseRequest,
                                                  rips::NNCConnections,
                                                  RiaNNCConnectionsStateHandler>( this,
                                                                                  &Self::GetNNCConnections,
                                                                                  &Self::RequestGetNNCConnections,
                                                                                  new RiaNNCConnectionsStateHandler ),
          new RiaGrpcServerToClientStreamCallback<Self,
                                                  NNCValuesRequest,
                                                  rips::NNCValues,
                                                  RiaNNCValuesStateHandler>( this,
                                                                             &Self::GetNNCValues,
                                                                             &Self::RequestGetNNCValues,
                                                                             new RiaNNCValuesStateHandler ),

          new RiaGrpcClientToServerStreamCallback<Self,
                                                  NNCValuesChunk,
                                                  ClientToServerStreamReply,
                                                  RiaNNCInputValuesStateHandler>( this,
                                                                                  &Self::SetNNCValues,
                                                                                  &Self::RequestSetNNCValues,
                                                                                  new RiaNNCInputValuesStateHandler(
                                                                                      true ) ) };

    return callbacks;
}

static bool RiaGrpcNNCPropertiesService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcNNCPropertiesService>(
        typeid( RiaGrpcNNCPropertiesService ).hash_code() );
