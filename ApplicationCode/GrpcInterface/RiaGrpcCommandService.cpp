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

#include "RiaGrpcServerCallData.h"
#include "cafPdmDefaultObjectFactory.h"

#include "RicfSetTimeStep.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCommandService::Execute(grpc::ServerContext* context, const CommandParams* request, Empty* reply)
{
    auto descriptor = request->descriptor();
    RiaLogging::info(QString::fromStdString(descriptor->name()));
    int numFields = descriptor->field_count();
    
    CommandParams::ParamsCase paramsCase = request->params_case();
    if (paramsCase != CommandParams::PARAMS_NOT_SET)
    {
        auto field = descriptor->FindFieldByNumber((int) paramsCase);
        RiaLogging::info(QString("Found Command Parameters: %1").arg(QString::fromStdString(field->name())));
    }
    return Status::OK;
    /*

    //RicfSetTimeStep* timeStepCommand = dynamic_cast<RicfSetTimeStep*>(caf::PdmDefaultObjectFactory::instance()->create("setTimeStep"));
    if (timeStepCommand)
    {
        auto descriptor = request->descriptor();        
        RiaLogging::info(QString::fromStdString(descriptor->name()));
        int numFields = descriptor->field_count();
        RiaLogging::info(QString("Has %1").arg(numFields));
        for (int i = 0; i < numFields; ++i)
        {
            auto field = descriptor->FindFieldByNumber(i + 1);
            if (field)
            {
                RiaLogging::info(QString("Found field: %1").arg(QString::fromStdString(field->name())));
            }
        }

        timeStepCommand->setCaseId(request->case_id());
        timeStepCommand->setTimeStepIndex(request->time_step());
        timeStepCommand->execute();
        return Status::OK;
    }
    return grpc::Status(grpc::NOT_FOUND, "Time step command not found"); */
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcServerCallMethod*> RiaGrpcCommandService::createCallbacks()
{
    typedef RiaGrpcCommandService Self;

    return { new RiaGrpcServerCallData<Self, CommandParams, Empty>(this, &Self::Execute, &Self::RequestExecute) };
}

static bool RiaGrpcCommandService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcCommandService>(typeid(RiaGrpcCommandService).hash_code());
