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
grpc::Status RiaGrpcCommandService::Execute(grpc::ServerContext* context, const CommandParams* request, Empty* reply)
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
                        assignFieldValue(pdmValueFieldHandle, params, parameter);
                    }
                }
            }
            commandHandle->execute();

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

    return {new RiaGrpcCallback<Self, CommandParams, Empty>(this, &Self::Execute, &Self::RequestExecute)};
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcCommandService::assignFieldValue(caf::PdmValueField*    pdmValueField,
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
            qValue     = QVariant(value);
            break;
        }
        case FieldDescriptor::TYPE_UINT32: {
            uint value = params.GetReflection()->GetUInt32(params, paramDescriptor);
            qValue     = QVariant(value);
            break;
        }
        case FieldDescriptor::TYPE_INT64: {
            int64_t value = params.GetReflection()->GetInt64(params, paramDescriptor);
            qValue     = QVariant((qlonglong) value);
            break;
        }
        case FieldDescriptor::TYPE_UINT64: {
            uint64_t value = params.GetReflection()->GetUInt64(params, paramDescriptor);
            qValue     = QVariant((qulonglong) value);
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

static bool RiaGrpcCommandService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcCommandService>(typeid(RiaGrpcCommandService).hash_code());
