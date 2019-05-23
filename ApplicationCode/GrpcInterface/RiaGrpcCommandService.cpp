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

#include "RiaLogging.h"

#include "RiaGrpcCallbacks.h"

#include "RicfSetTimeStep.h"

#include "cafAssert.h"
#include "cafPdmDefaultObjectFactory.h"
#include "cafPdmValueField.h"

using namespace rips;
using namespace google::protobuf;

#ifdef WIN32
#ifdef GetMessage
#undef GetMessage
#endif
#endif

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCommandService::Execute(grpc::ServerContext* context, const CommandParams* request, CommandReply* reply)
{
    auto requestDescriptor = request->GetDescriptor();
    RiaLogging::info(QString::fromStdString(requestDescriptor->name()));

    CommandParams::ParamsCase paramsCase = request->params_case();
    if (paramsCase != CommandParams::PARAMS_NOT_SET)
    {
        auto grpcOneOfMessage = requestDescriptor->FindFieldByNumber((int)paramsCase);
        CAF_ASSERT(grpcOneOfMessage->type() == FieldDescriptor::TYPE_MESSAGE);

        const Message& params               = request->GetReflection()->GetMessage(*request, grpcOneOfMessage);
        QString        grpcOneOfMessageName = QString::fromStdString(grpcOneOfMessage->name());
        RiaLogging::info(QString("Found Command: %1").arg(grpcOneOfMessageName));
        auto pdmObjectHandle = caf::PdmDefaultObjectFactory::instance()->create(grpcOneOfMessageName);
        auto commandHandle   = dynamic_cast<RicfCommandObject*>(pdmObjectHandle);
        if (commandHandle)
        {
            auto subMessageDescriptor = grpcOneOfMessage->message_type();
            int  numParameters        = subMessageDescriptor->field_count();
            for (int i = 0; i < numParameters; ++i)
            {
                auto parameter = subMessageDescriptor->field(i);
                if (parameter)
                {
                    QString parameterName       = QString::fromStdString(parameter->name());
                    auto    pdmValueFieldHandle = dynamic_cast<caf::PdmValueField*>(pdmObjectHandle->findField(parameterName));
                    if (pdmValueFieldHandle)
                    {
                        RiaLogging::info(QString("Found Matching Parameter: %1").arg(parameterName));
                        assignPdmFieldValue(pdmValueFieldHandle, params, parameter);
                    }
                }
            }
            RicfCommandResponse response = commandHandle->execute();
            if (response.status() == RicfCommandResponse::COMMAND_ERROR)
            {
                return grpc::Status(grpc::FAILED_PRECONDITION, response.message().toStdString());
            }
            else if (response.status() == RicfCommandResponse::COMMAND_WARNING)
            {
                context->AddInitialMetadata("warning", response.message().toStdString());
            }

            assignResultToReply(response.result(), reply);

            return Status::OK;
        }
    }
    return grpc::Status(grpc::NOT_FOUND, "Command not found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaAbstractGrpcCallback*> RiaGrpcCommandService::createCallbacks()
{
    typedef RiaGrpcCommandService Self;

    return {new RiaGrpcCallback<Self, CommandParams, CommandReply>(this, &Self::Execute, &Self::RequestExecute)};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignPdmFieldValue(caf::PdmValueField*    pdmValueField,
                                                const Message&         params,
                                                const FieldDescriptor* paramDescriptor)
{
    FieldDescriptor::Type fieldDataType = paramDescriptor->type();
    QVariant              qValue;
    switch (fieldDataType)
    {
        case FieldDescriptor::TYPE_BOOL: {
            auto value = params.GetReflection()->GetBool(params, paramDescriptor);
            qValue     = QVariant(value);
            break;
        }
        case FieldDescriptor::TYPE_INT32: {
            int value = params.GetReflection()->GetInt32(params, paramDescriptor);
            qValue    = QVariant(value);
            break;
        }
        case FieldDescriptor::TYPE_UINT32: {
            uint value = params.GetReflection()->GetUInt32(params, paramDescriptor);
            qValue     = QVariant(value);
            break;
        }
        case FieldDescriptor::TYPE_INT64: {
            int64_t value = params.GetReflection()->GetInt64(params, paramDescriptor);
            qValue        = QVariant((qlonglong)value);
            break;
        }
        case FieldDescriptor::TYPE_UINT64: {
            uint64_t value = params.GetReflection()->GetUInt64(params, paramDescriptor);
            qValue         = QVariant((qulonglong)value);
            break;
        }
        case FieldDescriptor::TYPE_STRING: {
            auto value = params.GetReflection()->GetString(params, paramDescriptor);
            qValue     = QVariant(QString::fromStdString(value));
            break;
        }
        case FieldDescriptor::TYPE_FLOAT: {
            auto value = params.GetReflection()->GetFloat(params, paramDescriptor);
            qValue     = QVariant(value);
            break;
        }
        case FieldDescriptor::TYPE_DOUBLE: {
            auto value = params.GetReflection()->GetDouble(params, paramDescriptor);
            qValue     = QVariant(value);
            break;
        }
        case FieldDescriptor::TYPE_ENUM: {
            auto value = params.GetReflection()->GetEnumValue(params, paramDescriptor);
            qValue     = QVariant(value);
            break;
        }
    }
    pdmValueField->setFromQVariant(qValue);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignGrpcFieldValue(google::protobuf::Message*               reply,
                                                 const google::protobuf::FieldDescriptor* fieldDescriptor,
                                                 const caf::PdmValueField*                pdmValueField)
{
    FieldDescriptor::Type fieldDataType = fieldDescriptor->type();
    QVariant              qValue        = pdmValueField->toQVariant();
    switch (fieldDataType)
    {
        case FieldDescriptor::TYPE_BOOL: {
            reply->GetReflection()->SetBool(reply, fieldDescriptor, qValue.toBool());
            break;
        }
        case FieldDescriptor::TYPE_INT32: {
            reply->GetReflection()->SetInt32(reply, fieldDescriptor, qValue.toInt());
            break;
        }
        case FieldDescriptor::TYPE_UINT32: {
            reply->GetReflection()->SetUInt32(reply, fieldDescriptor, qValue.toUInt());
            break;
        }
        case FieldDescriptor::TYPE_INT64: {
            reply->GetReflection()->SetInt64(reply, fieldDescriptor, qValue.toLongLong());
            break;
        }
        case FieldDescriptor::TYPE_UINT64: {
            reply->GetReflection()->SetUInt64(reply, fieldDescriptor, qValue.toULongLong());
            break;
        }
        case FieldDescriptor::TYPE_STRING: {
            reply->GetReflection()->SetString(reply, fieldDescriptor, qValue.toString().toStdString());
            break;
        }
        case FieldDescriptor::TYPE_FLOAT: {
            reply->GetReflection()->SetFloat(reply, fieldDescriptor, qValue.toFloat());
            break;
        }
        case FieldDescriptor::TYPE_DOUBLE: {
            reply->GetReflection()->SetDouble(reply, fieldDescriptor, qValue.toDouble());
            break;
        }
        case FieldDescriptor::TYPE_ENUM: {
            reply->GetReflection()->SetEnumValue(reply, fieldDescriptor, qValue.toInt());
            break;
        }
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignResultToReply(const caf::PdmObject* result, CommandReply* reply)
{
    if (!result)
    {
        reply->set_allocated_emptyresult(new Empty);
        return;
    }

    QString resultType = result->classKeyword();

    auto                   replyDescriptor = reply->GetDescriptor();
    auto                   oneofDescriptor = replyDescriptor->FindOneofByName("result");
    const FieldDescriptor* matchingOneOf   = nullptr;
    for (int fieldIndex = 0; fieldIndex < oneofDescriptor->field_count(); ++fieldIndex)
    {
        auto fieldDescriptor = oneofDescriptor->field(fieldIndex);
        if (fieldDescriptor->name() == resultType.toStdString())
        {
            matchingOneOf = fieldDescriptor;
            break;
        }
    }

    CAF_ASSERT(matchingOneOf);
    Message* message = reply->GetReflection()->MutableMessage(reply, matchingOneOf);
    CAF_ASSERT(message);
    auto resultDescriptor = message->GetDescriptor();

    for (int fieldIndex = 0; fieldIndex < resultDescriptor->field_count(); ++fieldIndex)
    {
        auto       fieldDescriptor = resultDescriptor->field(fieldIndex);
        const auto pdmField =
            dynamic_cast<const caf::PdmValueField*>(result->findField(QString::fromStdString(fieldDescriptor->name())));
        assignGrpcFieldValue(message, fieldDescriptor, pdmField);
    }
}

static bool RiaGrpcCommandService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcCommandService>(typeid(RiaGrpcCommandService).hash_code());
