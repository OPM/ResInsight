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
#include "RiaGrpcCommandService.h"

#include "RiaGrpcCallbacks.h"

#include "RicfReplaceCase.h"
#include "RicfSetTimeStep.h"

#include "cafAssert.h"
#include "cafPdmChildField.h"
#include "cafPdmDataValueField.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmObject.h"
#include "cafPdmValueField.h"
#include <google/protobuf/reflection.h>

using namespace rips;
using namespace google::protobuf;

// Windows may define GetMessage as a Macro and this is in direct conflict with the gRPC GetMessage calls.
#ifdef WIN32
#ifdef GetMessage
#undef GetMessage
#endif
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCommandService::Execute( grpc::ServerContext* context, const CommandParams* request, CommandReply* reply )
{
    auto requestDescriptor = request->GetDescriptor();

    CommandParams::ParamsCase paramsCase = request->params_case();
    if ( paramsCase != CommandParams::PARAMS_NOT_SET )
    {
        auto grpcOneOfMessage = requestDescriptor->FindFieldByNumber( (int)paramsCase );
        CAF_ASSERT( grpcOneOfMessage->type() == FieldDescriptor::TYPE_MESSAGE );

        QString grpcOneOfMessageName = QString::fromStdString( grpcOneOfMessage->name() );
        auto    pdmObjectHandle      = caf::PdmDefaultObjectFactory::instance()->create( grpcOneOfMessageName );
        auto    commandHandle        = dynamic_cast<RicfCommandObject*>( pdmObjectHandle );

        if ( commandHandle )
        {
            // Copy parameters
            RicfMultiCaseReplace* multiCaseReplaceCommand = dynamic_cast<RicfMultiCaseReplace*>( commandHandle );
            if ( multiCaseReplaceCommand )
            {
                CAF_ASSERT( request->has_replacemultiplecases() );
                auto                   replaceMultipleCasesRequest = request->replacemultiplecases();
                std::map<int, QString> caseIdFileMap;
                for ( auto caseGridFilePair : replaceMultipleCasesRequest.casepairs() )
                {
                    caseIdFileMap.insert( std::make_pair( caseGridFilePair.caseid(),
                                                          QString::fromStdString( caseGridFilePair.newgridfile() ) ) );
                }
                multiCaseReplaceCommand->setCaseReplacePairs( caseIdFileMap );
            }
            else
            {
                assignPdmObjectValues( commandHandle, *request, grpcOneOfMessage );
            }

            // Execute command
            caf::PdmScriptResponse response = commandHandle->execute();

            // Copy results
            if ( response.status() == caf::PdmScriptResponse::COMMAND_ERROR )
            {
                return grpc::Status( grpc::FAILED_PRECONDITION, response.sanitizedResponseMessage().toStdString() );
            }
            else if ( response.status() == caf::PdmScriptResponse::COMMAND_WARNING )
            {
                context->AddTrailingMetadata( "warning", response.sanitizedResponseMessage().toStdString() );
            }

            assignResultToReply( response.result(), reply );

            return Status::OK;
        }
    }
    return grpc::Status( grpc::NOT_FOUND, "Command not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcCommandService::createCallbacks()
{
    typedef RiaGrpcCommandService Self;

    return { new RiaGrpcUnaryCallback<Self, CommandParams, CommandReply>( this, &Self::Execute, &Self::RequestExecute ) };
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
caf::PdmField<T>* RiaGrpcCommandService::dataValueField( caf::PdmValueField* valueField )
{
    caf::PdmField<T>* dataValField = dynamic_cast<caf::PdmField<T>*>( valueField );
    CAF_ASSERT( dataValField );
    return dataValField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
template <typename T>
const caf::PdmField<T>* RiaGrpcCommandService::constDataValueField( const caf::PdmValueField* valueField )
{
    const caf::PdmField<T>* dataValField = dynamic_cast<const caf::PdmField<T>*>( valueField );
    CAF_ASSERT( dataValField );
    return dataValField;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignPdmFieldValue( caf::PdmValueField*    pdmValueField,
                                                 const Message&         params,
                                                 const FieldDescriptor* paramDescriptor )
{
    FieldDescriptor::Type fieldDataType = paramDescriptor->type();
    const Reflection*     reflection    = params.GetReflection();

    if ( paramDescriptor->is_repeated() && fieldDataType != FieldDescriptor::TYPE_INT32 &&
         fieldDataType != FieldDescriptor::TYPE_STRING )
    {
        CAF_ASSERT( false && "Only integer and string vectors are implemented as command arguments" );
    }

    switch ( fieldDataType )
    {
        case FieldDescriptor::TYPE_BOOL:
        {
            auto value     = reflection->GetBool( params, paramDescriptor );
            auto dataField = dataValueField<bool>( pdmValueField );
            dataField->setValue( value );
            break;
        }
        case FieldDescriptor::TYPE_INT32:
        {
            if ( paramDescriptor->is_repeated() )
            {
                RepeatedFieldRef<int> repeatedField = reflection->GetRepeatedFieldRef<int>( params, paramDescriptor );
                auto                  dataField     = dataValueField<std::vector<int>>( pdmValueField );
                dataField->setValue( std::vector<int>( repeatedField.begin(), repeatedField.end() ) );
            }
            else
            {
                int  value     = reflection->GetInt32( params, paramDescriptor );
                auto dataField = dataValueField<int>( pdmValueField );
                dataField->setValue( value );
            }
            break;
        }
        case FieldDescriptor::TYPE_UINT32:
        {
            uint value     = reflection->GetUInt32( params, paramDescriptor );
            auto dataField = dataValueField<uint>( pdmValueField );
            dataField->setValue( value );
            break;
        }
        case FieldDescriptor::TYPE_STRING:
        {
            if ( paramDescriptor->is_repeated() )
            {
                RepeatedFieldRef<std::string> repeatedField =
                    reflection->GetRepeatedFieldRef<std::string>( params, paramDescriptor );
                std::vector<QString> stringVector;
                for ( const std::string& string : repeatedField )
                {
                    stringVector.push_back( QString::fromStdString( string ) );
                }
                auto dataField = dataValueField<std::vector<QString>>( pdmValueField );
                dataField->setValue( stringVector );
            }
            else
            {
                auto value     = QString::fromStdString( reflection->GetString( params, paramDescriptor ) );
                auto dataField = dataValueField<QString>( pdmValueField );
                dataField->setValue( value );
            }
            break;
        }
        case FieldDescriptor::TYPE_FLOAT:
        {
            auto value     = reflection->GetFloat( params, paramDescriptor );
            auto dataField = dataValueField<float>( pdmValueField );
            dataField->setValue( value );
            break;
        }
        case FieldDescriptor::TYPE_DOUBLE:
        {
            auto value     = reflection->GetDouble( params, paramDescriptor );
            auto dataField = dataValueField<double>( pdmValueField );
            dataField->setValue( value );
            break;
        }
        case FieldDescriptor::TYPE_ENUM:
        {
            auto value = reflection->GetEnumValue( params, paramDescriptor );
            pdmValueField->setFromQVariant( QVariant( value ) );
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignPdmObjectValues( caf::PdmObjectHandle*                    pdmObjectHandle,
                                                   const google::protobuf::Message&         params,
                                                   const google::protobuf::FieldDescriptor* paramDescriptor )
{
    FieldDescriptor::Type fieldDataType = paramDescriptor->type();
    const Reflection*     reflection    = params.GetReflection();

    CAF_ASSERT( fieldDataType == FieldDescriptor::TYPE_MESSAGE );

    const Message& subMessage = reflection->GetMessage( params, paramDescriptor );

    const rips::PdmObject* ripsPdmObject = dynamic_cast<const rips::PdmObject*>( &subMessage );
    if ( ripsPdmObject )
    {
        copyPdmObjectFromRipsToCaf( ripsPdmObject, pdmObjectHandle );
        return;
    }

    auto messageDescriptor = paramDescriptor->message_type();
    int  numParameters     = messageDescriptor->field_count();
    for ( int i = 0; i < numParameters; ++i )
    {
        auto parameter = messageDescriptor->field( i );
        if ( parameter )
        {
            QString parameterName = QString::fromStdString( parameter->name() );
            auto    pdmChildFieldHandle =
                dynamic_cast<caf::PdmChildFieldHandle*>( pdmObjectHandle->findField( parameterName ) );
            auto pdmValueFieldHandle = dynamic_cast<caf::PdmValueField*>( pdmObjectHandle->findField( parameterName ) );
            if ( pdmChildFieldHandle )
            {
                std::vector<caf::PdmObjectHandle*> childObjects;
                pdmChildFieldHandle->childObjects( &childObjects );
                caf::PdmObjectHandle* childObject = nullptr;
                CAF_ASSERT( childObjects.size() <= 1u ); // We do not support child array fields yet

                if ( childObjects.size() == 1u )
                {
                    childObject = childObjects.front();
                }
                else if ( childObjects.empty() )
                {
                    childObject = emplaceChildField( pdmChildFieldHandle, "" );
                }
                CAF_ASSERT( childObject );
                if ( childObject )
                {
                    assignPdmObjectValues( childObject, subMessage, parameter );
                }
            }
            else if ( pdmValueFieldHandle )
            {
                assignPdmFieldValue( pdmValueFieldHandle, subMessage, parameter );
            }
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignGrpcFieldValue( Message*                  reply,
                                                  const FieldDescriptor*    fieldDescriptor,
                                                  const caf::PdmValueField* pdmValueField )
{
    if ( fieldDescriptor->is_repeated() )
    {
        auto     reflection = reply->GetReflection();
        QVariant qValue     = pdmValueField->toQVariant();
        if ( fieldDescriptor->type() == FieldDescriptor::TYPE_STRING )
        {
            MutableRepeatedFieldRef<std::string> repeatedField =
                reflection->GetMutableRepeatedFieldRef<std::string>( reply, fieldDescriptor );
            QStringList stringList = qValue.toStringList();
            for ( QString stringValue : stringList )
            {
                repeatedField.Add( stringValue.toStdString() );
            }
        }
        else
        {
            CAF_ASSERT( false && "Assigning vector results to Command Results is only implemented for strings" );
        }
        return;
    }

    FieldDescriptor::Type fieldDataType = fieldDescriptor->type();
    QVariant              qValue        = pdmValueField->toQVariant();

    auto reflection = reply->GetReflection();
    switch ( fieldDataType )
    {
        case FieldDescriptor::TYPE_BOOL:
        {
            reflection->SetBool( reply, fieldDescriptor, qValue.toBool() );
            break;
        }
        case FieldDescriptor::TYPE_INT32:
        {
            reflection->SetInt32( reply, fieldDescriptor, qValue.toInt() );
            break;
        }
        case FieldDescriptor::TYPE_UINT32:
        {
            reflection->SetUInt32( reply, fieldDescriptor, qValue.toUInt() );
            break;
        }
        case FieldDescriptor::TYPE_INT64:
        {
            reflection->SetInt64( reply, fieldDescriptor, qValue.toLongLong() );
            break;
        }
        case FieldDescriptor::TYPE_UINT64:
        {
            reflection->SetUInt64( reply, fieldDescriptor, qValue.toULongLong() );
            break;
        }
        case FieldDescriptor::TYPE_STRING:
        {
            reflection->SetString( reply, fieldDescriptor, qValue.toString().toStdString() );
            break;
        }
        case FieldDescriptor::TYPE_FLOAT:
        {
            reflection->SetFloat( reply, fieldDescriptor, qValue.toFloat() );
            break;
        }
        case FieldDescriptor::TYPE_DOUBLE:
        {
            reflection->SetDouble( reply, fieldDescriptor, qValue.toDouble() );
            break;
        }
        case FieldDescriptor::TYPE_ENUM:
        {
            reflection->SetEnumValue( reply, fieldDescriptor, qValue.toInt() );
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignResultToReply( const caf::PdmObject* result, CommandReply* reply )
{
    if ( !result )
    {
        reply->set_allocated_emptyresult( new Empty );
        return;
    }

    QString resultType = result->classKeyword();

    auto                   replyDescriptor = reply->GetDescriptor();
    auto                   oneofDescriptor = replyDescriptor->FindOneofByName( "result" );
    const FieldDescriptor* matchingOneOf   = nullptr;
    for ( int fieldIndex = 0; fieldIndex < oneofDescriptor->field_count(); ++fieldIndex )
    {
        auto fieldDescriptor = oneofDescriptor->field( fieldIndex );
        if ( fieldDescriptor->name() == resultType.toStdString() )
        {
            matchingOneOf = fieldDescriptor;
            break;
        }
    }

    CAF_ASSERT( matchingOneOf );
    Message* message = reply->GetReflection()->MutableMessage( reply, matchingOneOf );
    CAF_ASSERT( message );
    auto resultDescriptor = message->GetDescriptor();

    for ( int fieldIndex = 0; fieldIndex < resultDescriptor->field_count(); ++fieldIndex )
    {
        auto       fieldDescriptor = resultDescriptor->field( fieldIndex );
        const auto pdmField        = dynamic_cast<const caf::PdmValueField*>(
            result->findField( QString::fromStdString( fieldDescriptor->name() ) ) );
        assignGrpcFieldValue( message, fieldDescriptor, pdmField );
    }
}

static bool RiaGrpcCommandService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcCommandService>(
    typeid( RiaGrpcCommandService ).hash_code() );
