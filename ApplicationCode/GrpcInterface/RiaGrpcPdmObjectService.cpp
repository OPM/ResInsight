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
#include "RiaGrpcPdmObjectService.h"

#include "RiaGrpcCallbacks.h"
#include "Rim3dView.h"
#include "RimEclipseResultDefinition.h"
#include "RimProject.h"

#include "cafPdmAbstractFieldScriptingCapability.h"
#include "cafPdmObject.h"
#include "cafPdmObjectMethod.h"
#include "cafPdmObjectScriptingCapability.h"
#include "cafPdmObjectScriptingCapabilityRegister.h"

using namespace rips;

template <typename DataType>
struct DataHolder : public AbstractDataHolder
{
    DataHolder( const DataType& data )
        : data( data )
    {
    }

    size_t dataCount() const override { return data.size(); }
    size_t dataSizeOf() const override { return sizeof( typename DataType::value_type ); }

    void   reserveReplyStorage( rips::PdmObjectGetterReply* reply ) const;
    void   addValueToReply( size_t valueIndex, rips::PdmObjectGetterReply* reply ) const;
    size_t getValuesFromChunk( size_t startIndex, const rips::PdmObjectSetterChunk* chunk );
    void   applyValuesToProxyField( caf::PdmProxyFieldHandle* proxyField );

    DataType data;
};

template <>
void DataHolder<std::vector<int>>::reserveReplyStorage( rips::PdmObjectGetterReply* reply ) const
{
    reply->mutable_ints()->mutable_data()->Reserve( data.size() );
}
template <>
void DataHolder<std::vector<int>>::addValueToReply( size_t valueIndex, rips::PdmObjectGetterReply* reply ) const
{
    reply->mutable_ints()->add_data( data[valueIndex] );
}
template <>
size_t DataHolder<std::vector<int>>::getValuesFromChunk( size_t startIndex, const rips::PdmObjectSetterChunk* chunk )
{
    size_t chunkSize    = chunk->ints().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->ints().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<std::vector<int>>::applyValuesToProxyField( caf::PdmProxyFieldHandle* proxyField )
{
    auto proxyValueField = dynamic_cast<caf::PdmProxyValueField<std::vector<int>>*>( proxyField );
    CAF_ASSERT( proxyValueField );
    if ( proxyValueField )
    {
        proxyValueField->setValue( data );
    }
}

template <>
void DataHolder<std::vector<double>>::reserveReplyStorage( rips::PdmObjectGetterReply* reply ) const
{
    reply->mutable_doubles()->mutable_data()->Reserve( data.size() );
}
template <>
void DataHolder<std::vector<double>>::addValueToReply( size_t valueIndex, rips::PdmObjectGetterReply* reply ) const
{
    reply->mutable_doubles()->add_data( data[valueIndex] );
}
template <>
size_t DataHolder<std::vector<double>>::getValuesFromChunk( size_t startIndex, const rips::PdmObjectSetterChunk* chunk )
{
    size_t chunkSize    = chunk->doubles().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = chunk->doubles().data()[chunkIndex];
    }
    return chunkSize;
}
template <>
void DataHolder<std::vector<double>>::applyValuesToProxyField( caf::PdmProxyFieldHandle* proxyField )
{
    auto proxyValueField = dynamic_cast<caf::PdmProxyValueField<std::vector<double>>*>( proxyField );
    CAF_ASSERT( proxyValueField );
    if ( proxyValueField )
    {
        proxyValueField->setValue( data );
    }
}

template <>
void DataHolder<std::vector<QString>>::reserveReplyStorage( rips::PdmObjectGetterReply* reply ) const
{
    reply->mutable_strings()->mutable_data()->Reserve( data.size() );
}
template <>
void DataHolder<std::vector<QString>>::addValueToReply( size_t valueIndex, rips::PdmObjectGetterReply* reply ) const
{
    reply->mutable_strings()->add_data( data[valueIndex].toStdString() );
}
template <>
size_t DataHolder<std::vector<QString>>::getValuesFromChunk( size_t startIndex, const rips::PdmObjectSetterChunk* chunk )
{
    size_t chunkSize    = chunk->strings().data_size();
    size_t currentIndex = startIndex;
    size_t chunkIndex   = 0u;
    for ( ; chunkIndex < chunkSize && currentIndex < data.size(); ++currentIndex, ++chunkIndex )
    {
        data[currentIndex] = QString::fromStdString( chunk->strings().data()[chunkIndex] );
    }
    return chunkSize;
}
template <>
void DataHolder<std::vector<QString>>::applyValuesToProxyField( caf::PdmProxyFieldHandle* proxyField )
{
    auto proxyValueField = dynamic_cast<caf::PdmProxyValueField<std::vector<QString>>*>( proxyField );
    CAF_ASSERT( proxyValueField );
    if ( proxyValueField )
    {
        proxyValueField->setValue( data );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaPdmObjectMethodStateHandler::RiaPdmObjectMethodStateHandler( bool clientToServerStreamer )
    : m_fieldOwner( nullptr )
    , m_proxyField( nullptr )
    , m_currentDataIndex( 0u )
    , m_clientToServerStreamer( clientToServerStreamer )
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaPdmObjectMethodStateHandler::init( const rips::PdmObjectGetterRequest* request )
{
    CAF_ASSERT( !m_clientToServerStreamer );
    m_fieldOwner      = RiaGrpcPdmObjectService::findCafObjectFromRipsObject( request->object() );
    QString fieldName = QString::fromStdString( request->method() );

    std::vector<caf::PdmFieldHandle*> fields;
    m_fieldOwner->fields( fields );
    for ( auto field : fields )
    {
        auto scriptability = field->capability<caf::PdmAbstractFieldScriptingCapability>();
        if ( scriptability && scriptability->scriptFieldName() == fieldName )
        {
            caf::PdmProxyFieldHandle* proxyField = dynamic_cast<caf::PdmProxyFieldHandle*>( field );
            if ( proxyField )
            {
                m_proxyField = proxyField;

                if ( dynamic_cast<caf::PdmProxyValueField<std::vector<int>>*>( field ) )
                {
                    auto dataField = dynamic_cast<caf::PdmProxyValueField<std::vector<int>>*>( field );
                    m_dataHolder.reset( new DataHolder<std::vector<int>>( dataField->value() ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<caf::PdmProxyValueField<std::vector<double>>*>( field ) )
                {
                    auto dataField = dynamic_cast<caf::PdmProxyValueField<std::vector<double>>*>( field );
                    m_dataHolder.reset( new DataHolder<std::vector<double>>( dataField->value() ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<caf::PdmProxyValueField<std::vector<QString>>*>( field ) )
                {
                    auto dataField = dynamic_cast<caf::PdmProxyValueField<std::vector<QString>>*>( field );
                    m_dataHolder.reset( new DataHolder<std::vector<QString>>( dataField->value() ) );
                    return grpc::Status::OK;
                }
                else
                {
                    CAF_ASSERT( false && "The proxy field data type is not yet supported for streaming fields" );
                }
            }
        }
    }

    return grpc::Status( grpc::NOT_FOUND, "Proxy field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaPdmObjectMethodStateHandler::init( const rips::PdmObjectSetterChunk* chunk )
{
    CAF_ASSERT( m_clientToServerStreamer );
    CAF_ASSERT( chunk->has_set_request() );
    auto setRequest    = chunk->set_request();
    auto methodRequest = setRequest.request();
    m_fieldOwner       = RiaGrpcPdmObjectService::findCafObjectFromRipsObject( methodRequest.object() );
    QString fieldName  = QString::fromStdString( methodRequest.method() );
    int     valueCount = setRequest.data_count();

    std::vector<caf::PdmFieldHandle*> fields;
    m_fieldOwner->fields( fields );
    for ( auto field : fields )
    {
        auto scriptability = field->capability<caf::PdmAbstractFieldScriptingCapability>();
        if ( scriptability && scriptability->scriptFieldName() == fieldName )
        {
            caf::PdmProxyFieldHandle* proxyField = dynamic_cast<caf::PdmProxyFieldHandle*>( field );
            if ( proxyField )
            {
                m_proxyField = proxyField;

                if ( dynamic_cast<caf::PdmProxyValueField<std::vector<int>>*>( field ) )
                {
                    m_dataHolder.reset( new DataHolder<std::vector<int>>( std::vector<int>( valueCount ) ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<caf::PdmProxyValueField<std::vector<double>>*>( field ) )
                {
                    m_dataHolder.reset( new DataHolder<std::vector<double>>( std::vector<double>( valueCount ) ) );
                    return grpc::Status::OK;
                }
                else if ( dynamic_cast<caf::PdmProxyValueField<std::vector<QString>>*>( field ) )
                {
                    m_dataHolder.reset( new DataHolder<std::vector<QString>>( std::vector<QString>( valueCount ) ) );
                    return grpc::Status::OK;
                }
                else
                {
                    CAF_ASSERT( false && "The proxy field data type is not yet supported for streaming fields" );
                }
            }
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Proxy field not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaPdmObjectMethodStateHandler::assignReply( rips::PdmObjectGetterReply* reply )
{
    CAF_ASSERT( m_dataHolder );
    const size_t packageSize    = RiaGrpcServiceInterface::numberOfDataUnitsInPackage( m_dataHolder->dataSizeOf() );
    size_t       indexInPackage = 0u;
    m_dataHolder->reserveReplyStorage( reply );

    for ( ; indexInPackage < packageSize && m_currentDataIndex < m_dataHolder->dataCount(); ++indexInPackage )
    {
        m_dataHolder->addValueToReply( m_currentDataIndex, reply );
        m_currentDataIndex++;
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
Status RiaPdmObjectMethodStateHandler::receiveRequest( const rips::PdmObjectSetterChunk* chunk,
                                                       rips::ClientToServerStreamReply*  reply )
{
    size_t valuesWritten = m_dataHolder->getValuesFromChunk( m_currentDataIndex, chunk );
    m_currentDataIndex += valuesWritten;

    if ( m_currentDataIndex > totalValueCount() )
    {
        return grpc::Status( grpc::OUT_OF_RANGE, "Attempting to write out of bounds" );
    }
    reply->set_accepted_value_count( static_cast<int64_t>( m_currentDataIndex ) );
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaPdmObjectMethodStateHandler::streamedValueCount() const
{
    return m_currentDataIndex;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
size_t RiaPdmObjectMethodStateHandler::totalValueCount() const
{
    return m_dataHolder->dataCount();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaPdmObjectMethodStateHandler::finish()
{
    if ( m_proxyField )
    {
        QVariant before = m_proxyField->toQVariant();
        m_dataHolder->applyValuesToProxyField( m_proxyField );
        QVariant after = m_proxyField->toQVariant();
        m_fieldOwner->fieldChangedByUi( m_proxyField, before, after );
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::GetAncestorPdmObject( grpc::ServerContext*                context,
                                                            const rips::PdmParentObjectRequest* request,
                                                            rips::PdmObject*                    reply )
{
    RimProject*                  project = RimProject::current();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    QString scriptClassName = QString::fromStdString( request->object().class_keyword() );
    QString classKeyword = caf::PdmObjectScriptingCapabilityRegister::classKeywordFromScriptClassName( scriptClassName );

    project->descendantsIncludingThisFromClassKeyword( classKeyword, objectsOfCurrentClass );

    caf::PdmObject* matchingObject = nullptr;
    for ( caf::PdmObject* testObject : objectsOfCurrentClass )
    {
        if ( reinterpret_cast<uint64_t>( testObject ) == request->object().address() )
        {
            matchingObject = testObject;
        }
    }

    if ( matchingObject )
    {
        caf::PdmObject* parentObject       = nullptr;
        QString         ancestorScriptName = QString::fromStdString( request->parent_keyword() );
        QString         ancestorClassKeyword =
            caf::PdmObjectScriptingCapabilityRegister::classKeywordFromScriptClassName( ancestorScriptName );
        matchingObject->firstAncestorOrThisFromClassKeyword( ancestorClassKeyword, parentObject );
        if ( parentObject )
        {
            copyPdmObjectFromCafToRips( parentObject, reply );
            return grpc::Status::OK;
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Parent PdmObject not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::GetDescendantPdmObjects( grpc::ServerContext*                    context,
                                                               const rips::PdmDescendantObjectRequest* request,
                                                               rips::PdmObjectArray*                   reply )
{
    auto matchingObject = findCafObjectFromRipsObject( request->object() );

    if ( matchingObject )
    {
        std::vector<caf::PdmObject*> childObjects;
        QString childClassKeyword = caf::PdmObjectScriptingCapabilityRegister::classKeywordFromScriptClassName(
            QString::fromStdString( request->child_keyword() ) );
        matchingObject->descendantsIncludingThisFromClassKeyword( childClassKeyword, childObjects );
        for ( auto pdmChild : childObjects )
        {
            rips::PdmObject* ripsChild = reply->add_objects();
            copyPdmObjectFromCafToRips( pdmChild, ripsChild );
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Current PdmObject not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::GetChildPdmObjects( grpc::ServerContext*               context,
                                                          const rips::PdmChildObjectRequest* request,
                                                          rips::PdmObjectArray*              reply )
{
    auto matchingObject = findCafObjectFromRipsObject( request->object() );
    if ( matchingObject )
    {
        QString                           fieldName = QString::fromStdString( request->child_field() );
        std::vector<caf::PdmFieldHandle*> fields;
        matchingObject->fields( fields );
        for ( auto field : fields )
        {
            auto scriptability = field->capability<caf::PdmAbstractFieldScriptingCapability>();
            if ( scriptability && scriptability->scriptFieldName() == fieldName )
            {
                std::vector<caf::PdmObjectHandle*> childObjects;
                field->childObjects( &childObjects );
                for ( auto pdmChild : childObjects )
                {
                    rips::PdmObject* ripsChild = reply->add_objects();
                    copyPdmObjectFromCafToRips( static_cast<caf::PdmObject*>( pdmChild ), ripsChild );
                }
                return grpc::Status::OK;
            }
        }
        return grpc::Status( grpc::NOT_FOUND, "Child field not found" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Current PdmObject not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::UpdateExistingPdmObject( grpc::ServerContext*   context,
                                                               const rips::PdmObject* request,
                                                               rips::Empty*           response )
{
    auto matchingObject = findCafObjectFromRipsObject( *request );

    if ( matchingObject )
    {
        copyPdmObjectFromRipsToCaf( request, matchingObject );
        RimEclipseResultDefinition* resultDefinition = dynamic_cast<RimEclipseResultDefinition*>( matchingObject );
        // TODO: Make this more general. Perhaps we need an interface method for updating UI fields
        if ( resultDefinition )
        {
            resultDefinition->updateUiFieldsFromActiveResult();
            resultDefinition->loadResult();
        }

        matchingObject->updateAllRequiredEditors();
        RimProject::current()->scheduleCreateDisplayModelAndRedrawAllViews();

        Rim3dView* view = dynamic_cast<Rim3dView*>( matchingObject );
        if ( view )
        {
            view->applyBackgroundColorAndFontChanges();
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "PdmObject not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::CreateChildPdmObject( grpc::ServerContext*                     context,
                                                            const rips::CreatePdmChildObjectRequest* request,
                                                            rips::PdmObject*                         reply )
{
    auto matchingObject = findCafObjectFromRipsObject( request->object() );

    if ( matchingObject )
    {
        CAF_ASSERT( request );

        caf::PdmObjectHandle* pdmObject =
            emplaceChildField( matchingObject, QString::fromStdString( request->child_field() ) );
        if ( pdmObject )
        {
            copyPdmObjectFromCafToRips( pdmObject, reply );
            return grpc::Status::OK;
        }
        return grpc::Status( grpc::NOT_FOUND, "Could not create PdmObject" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Could not find PdmObject" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::CallPdmObjectGetter( grpc::ServerContext*                context,
                                                           const rips::PdmObjectGetterRequest* request,
                                                           rips::PdmObjectGetterReply*         reply,
                                                           RiaPdmObjectMethodStateHandler*     stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::CallPdmObjectSetter( grpc::ServerContext*              context,
                                                           const rips::PdmObjectSetterChunk* chunk,
                                                           rips::ClientToServerStreamReply*  reply,
                                                           RiaPdmObjectMethodStateHandler*   stateHandler )
{
    return stateHandler->receiveRequest( chunk, reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::CallPdmObjectMethod( grpc::ServerContext*                context,
                                                           const rips::PdmObjectMethodRequest* request,
                                                           rips::PdmObject*                    reply )
{
    auto matchingObject = findCafObjectFromRipsObject( request->object() );
    if ( matchingObject )
    {
        QString methodKeyword = QString::fromStdString( request->method() );

        std::shared_ptr<caf::PdmObjectMethod> method =
            caf::PdmObjectMethodFactory::instance()->createMethod( matchingObject, methodKeyword );
        if ( method )
        {
            copyPdmObjectFromRipsToCaf( &( request->params() ), method.get() );

            caf::PdmObjectHandle* result = method->execute();
            if ( result )
            {
                copyPdmObjectFromCafToRips( result, reply );
                if ( !method->resultIsPersistent() )
                {
                    delete result;
                }
                return grpc::Status::OK;
            }
            else
            {
                if ( method->isNullptrValidResult() )
                {
                    return grpc::Status::OK;
                }

                return grpc::Status( grpc::NOT_FOUND, "No result returned from Method" );
            }
        }
        return grpc::Status( grpc::NOT_FOUND, "Could not find Method" );
    }
    return grpc::Status( grpc::NOT_FOUND, "Could not find PdmObject" );
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcPdmObjectService::createCallbacks()
{
    typedef RiaGrpcPdmObjectService Self;
    return {
        new RiaGrpcUnaryCallback<Self, PdmParentObjectRequest, PdmObject>( this,
                                                                           &Self::GetAncestorPdmObject,
                                                                           &Self::RequestGetAncestorPdmObject ),
        new RiaGrpcUnaryCallback<Self, PdmDescendantObjectRequest, PdmObjectArray>( this,
                                                                                    &Self::GetDescendantPdmObjects,
                                                                                    &Self::RequestGetDescendantPdmObjects ),
        new RiaGrpcUnaryCallback<Self, PdmChildObjectRequest, PdmObjectArray>( this,
                                                                               &Self::GetChildPdmObjects,
                                                                               &Self::RequestGetChildPdmObjects ),
        new RiaGrpcUnaryCallback<Self, PdmObject, Empty>( this,
                                                          &Self::UpdateExistingPdmObject,
                                                          &Self::RequestUpdateExistingPdmObject ),
        new RiaGrpcUnaryCallback<Self, CreatePdmChildObjectRequest, PdmObject>( this,
                                                                                &Self::CreateChildPdmObject,
                                                                                &Self::RequestCreateChildPdmObject ),
        new RiaGrpcServerToClientStreamCallback<Self,
                                                PdmObjectGetterRequest,
                                                PdmObjectGetterReply,
                                                RiaPdmObjectMethodStateHandler>( this,
                                                                                 &Self::CallPdmObjectGetter,
                                                                                 &Self::RequestCallPdmObjectGetter,
                                                                                 new RiaPdmObjectMethodStateHandler ),

        new RiaGrpcClientToServerStreamCallback<Self,
                                                PdmObjectSetterChunk,
                                                ClientToServerStreamReply,
                                                RiaPdmObjectMethodStateHandler>( this,
                                                                                 &Self::CallPdmObjectSetter,
                                                                                 &Self::RequestCallPdmObjectSetter,
                                                                                 new RiaPdmObjectMethodStateHandler( true ) ),
        new RiaGrpcUnaryCallback<Self, PdmObjectMethodRequest, PdmObject>( this,
                                                                           &Self::CallPdmObjectMethod,
                                                                           &Self::RequestCallPdmObjectMethod ),

    };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiaGrpcPdmObjectService::findCafObjectFromRipsObject( const rips::PdmObject& ripsObject )
{
    QString  scriptClassName = QString::fromStdString( ripsObject.class_keyword() );
    uint64_t address         = ripsObject.address();
    return findCafObjectFromScriptNameAndAddress( scriptClassName, address );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
caf::PdmObject* RiaGrpcPdmObjectService::findCafObjectFromScriptNameAndAddress( const QString& scriptClassName,
                                                                                uint64_t       address )
{
    RimProject*                  project = RimProject::current();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    QString classKeyword = caf::PdmObjectScriptingCapabilityRegister::classKeywordFromScriptClassName( scriptClassName );

    project->descendantsIncludingThisFromClassKeyword( classKeyword, objectsOfCurrentClass );

    caf::PdmObject* matchingObject = nullptr;
    for ( caf::PdmObject* testObject : objectsOfCurrentClass )
    {
        if ( reinterpret_cast<uint64_t>( testObject ) == address )
        {
            matchingObject = testObject;
        }
    }
    return matchingObject;
}

static bool RiaGrpcPdmObjectService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcPdmObjectService>(
    typeid( RiaGrpcPdmObjectService ).hash_code() );
