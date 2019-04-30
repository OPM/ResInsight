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
#include "RiaGrpcServer.h"

#include "RiaApplication.h"
#include "RiaDefines.h"
#include "RigCaseCellResultsData.h"
#include "RigMainGrid.h"
#include "RimEclipseCase.h"
#include "RimProject.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RiaGridServiceImpl::getCase(int caseId) const
{
    std::vector<RimEclipseCase*> cases = RiaApplication::instance()->project()->eclipseCases();
    for (const auto& rimCase : cases)
    {
        if (rimCase->caseId() == caseId)
        {
            return rimCase;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGridServiceImpl::dimensions(ServerContext* context, const Case* request, Vec3i* reply)
{
    int caseId = request->id();
    std::cout << "Got dimension request for case id: " << caseId << std::endl;

    RimEclipseCase* rimCase = getCase(caseId);
    if (rimCase)
    {
        std::cout << "Sending back dimensions" << std::endl;
        reply->set_i((int)rimCase->mainGrid()->cellCountI());
        reply->set_j((int)rimCase->mainGrid()->cellCountJ());
        reply->set_k((int)rimCase->mainGrid()->cellCountK());
        return Status::OK;
    }
    return Status(grpc::NOT_FOUND, "Invalid case id");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGridServiceImpl::results(ServerContext* context, const EclipseResultRequest* request, DoubleResult* result)
{
    int caseId = request->result_case().id();
    std::cout << "Got result request for case id: " << caseId << std::endl;
    RimEclipseCase* rimCase = getCase(caseId);
    if (rimCase)
    {
        RigCaseCellResultsData* eclipseResults = rimCase->results(RiaDefines::MATRIX_MODEL);
        EclipseResultAddress    msgAddress     = request->result_address();
        int timeLapseStepFrameIdx = -1, difference_case_id = -1;
        if (msgAddress.has_time_lapse_frame())
        {
            timeLapseStepFrameIdx = msgAddress.time_lapse_frame().value();            
        }
        if (msgAddress.has_difference_case())
        {
            difference_case_id = msgAddress.difference_case().value();
        }

        RigEclipseResultAddress resAddress(RiaDefines::ResultCatType(msgAddress.result_cat_type()),
                                           QString::fromStdString(msgAddress.result_name()),
                                           timeLapseStepFrameIdx,
                                           difference_case_id);
        if (eclipseResults->hasResultEntry(resAddress))
        {
            size_t timeStep = (size_t)request->time_step();
            const std::vector<std::vector<double>>& resultValues = eclipseResults->cellScalarResults(resAddress);
            if (timeStep < resultValues.size())
            {
                result->mutable_value()->Reserve((int) resultValues[timeStep].size());
                for (size_t i = 0; i < resultValues[timeStep].size(); ++i)
                {
                    result->add_value(resultValues[timeStep][i]);
                    if (i == 25347) std::cout << "Result for cell " << i << ": " << resultValues[timeStep][i] << std::endl;
                }              
                std::cout << "Successfully Packed " << resultValues[timeStep].size() << " values" << std::endl;
                return Status::OK;
            }
            std::cout << "Invalid time step" << std::endl;
            return Status(grpc::NOT_FOUND, "Invalid Time Step");
        }
        std::cout << "Invalid result address" << std::endl;
        return Status(grpc::NOT_FOUND, "Invalid Result Address");
    }
    std::cout << "Invalid case id" << std::endl;
    return Status(grpc::NOT_FOUND, "Invalid case id");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGridServiceImpl::numberOfTimeSteps(ServerContext* context, const Case* request, Int32Message* reply)
{
    int caseId = request->id();
    std::cout << "Got time steps request for case id: " << caseId << std::endl;
    RimEclipseCase* rimCase = getCase(caseId);
    if (rimCase)
    {
        reply->set_value(rimCase->timeStepStrings().size());
        return Status::OK;
    }
    return Status(grpc::NOT_FOUND, "Invalid case id");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::runInOwnMainLoop()
{
    run();

    // Wait for the server to shutdown. Note that some other thread must be
    // responsible for shutting down the server for this call to ever return.
    server_->Wait();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::run()
{
    std::string        server_address("localhost:50051");
    RiaGridServiceImpl service;

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&service);
    // Finally assemble the server.
    server_ = builder.BuildAndStart();
    std::cout << "Server listening on " << server_address << std::endl;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::handleRequest()
{

}
