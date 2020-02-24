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

#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RicfObjectCapability.h"
#include "Rim3dView.h"
#include "RimEclipseResultDefinition.h"
#include "RimProject.h"

#include "cafPdmObject.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPdmObjectService::GetAncestorPdmObject( grpc::ServerContext*                context,
                                                            const rips::PdmParentObjectRequest* request,
                                                            rips::PdmObject*                    reply )
{
    RimProject*                  project = RiaApplication::instance()->project();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    QString scriptClassName = QString::fromStdString( request->object().class_keyword() );
    QString classKeyword    = RicfObjectCapability::classKeywordFromScriptClassName( scriptClassName );

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
        QString ancestorClassKeyword = RicfObjectCapability::classKeywordFromScriptClassName( ancestorScriptName );
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
    RimProject*                  project = RiaApplication::instance()->project();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    QString scriptClassName = QString::fromStdString( request->object().class_keyword() );
    QString classKeyword    = RicfObjectCapability::classKeywordFromScriptClassName( scriptClassName );

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
        std::vector<caf::PdmObject*> childObjects;
        QString                      childClassKeyword =
            RicfObjectCapability::classKeywordFromScriptClassName( QString::fromStdString( request->child_keyword() ) );
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
    RimProject*                  project = RiaApplication::instance()->project();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    QString scriptClassName = QString::fromStdString( request->object().class_keyword() );
    QString classKeyword    = RicfObjectCapability::classKeywordFromScriptClassName( scriptClassName );

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
        QString                           fieldName = QString::fromStdString( request->child_field() );
        std::vector<caf::PdmFieldHandle*> fields;
        matchingObject->fields( fields );
        for ( auto field : fields )
        {
            if ( field->keyword() == fieldName )
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
    RimProject*                  project = RiaApplication::instance()->project();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    QString scriptClassName = QString::fromStdString( request->class_keyword() );
    QString classKeyword    = RicfObjectCapability::classKeywordFromScriptClassName( scriptClassName );

    project->descendantsIncludingThisFromClassKeyword( classKeyword, objectsOfCurrentClass );

    caf::PdmObject* matchingObject = nullptr;
    for ( caf::PdmObject* pdmObject : objectsOfCurrentClass )
    {
        if ( reinterpret_cast<uint64_t>( pdmObject ) == request->address() )
        {
            matchingObject = pdmObject;
        }
    }

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
        project->scheduleCreateDisplayModelAndRedrawAllViews();
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
    RimProject*                  project = RiaApplication::instance()->project();
    std::vector<caf::PdmObject*> objectsOfCurrentClass;

    QString scriptClassName = QString::fromStdString( request->object().class_keyword() );
    QString classKeyword    = RicfObjectCapability::classKeywordFromScriptClassName( scriptClassName );

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
std::vector<RiaGrpcCallbackInterface*> RiaGrpcPdmObjectService::createCallbacks()
{
    typedef RiaGrpcPdmObjectService Self;
    return {new RiaGrpcUnaryCallback<Self, PdmParentObjectRequest, PdmObject>( this,
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
                                                                                    &Self::RequestCreateChildPdmObject )};
}

static bool RiaGrpcPdmObjectService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcPdmObjectService>(
    typeid( RiaGrpcPdmObjectService ).hash_code() );
