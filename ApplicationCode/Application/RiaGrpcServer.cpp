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
    RiaLogging::debug("Got dimension request");

    RimEclipseCase* rimCase = getCase(caseId);
    if (rimCase)
    {
        RiaLogging::debug("Sending back dimensions");
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
    RiaLogging::debug("GRPC: Got result request");
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
                }
                return Status::OK;
            }
            RiaLogging::error("GRPC: Invalid Time Step");
            return Status(grpc::NOT_FOUND, "Invalid Time Step");
        }
        RiaLogging::error("GRPC: Invalid Result Address");
        return Status(grpc::NOT_FOUND, "Invalid Result Address");
    }
    RiaLogging::error("GRPC: Invalid Case Id");
    return Status(grpc::NOT_FOUND, "Invalid case id");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
Status RiaGridServiceImpl::numberOfTimeSteps(ServerContext* context, const Case* request, Int32Message* reply)
{
    int caseId = request->id();
    RiaLogging::debug("GRPC: Got time steps request");

    RimEclipseCase* rimCase = getCase(caseId);
    if (rimCase)
    {
        reply->set_value(rimCase->timeStepStrings().size());
        return Status::OK;
    }
    RiaLogging::error("GRPC: Invalid Case Id");
    return Status(grpc::NOT_FOUND, "Invalid case id");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServer::~RiaGrpcServer()
{
    m_thread.join();
    m_server->Shutdown();
    m_completionQueue->Shutdown();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::run()
{
    initialize();
    while (true)
    {
        waitForNextRequest();
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::runInThread()
{
    initialize();
    m_thread = std::thread(&RiaGrpcServer::waitForNextRequest, this);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::initialize()
{
    std::string        server_address("localhost:50051");

    ServerBuilder builder;
    // Listen on the given address without any authentication mechanism.
    builder.AddListeningPort(server_address, grpc::InsecureServerCredentials());
    // Register "service" as the instance through which we'll communicate with
    // clients. In this case it corresponds to an *synchronous* service.
    builder.RegisterService(&m_service);

    m_completionQueue = builder.AddCompletionQueue();

    // Finally assemble the server.
    m_server = builder.BuildAndStart();

    RiaLogging::info(QString("Server listening on %1").arg(QString::fromStdString(server_address)));
    // Spawn new CallData instances to serve new clients.
    process(new RiaGrpcServerCallData<Case, Vec3i>("dimensions"));
    process(new RiaGrpcServerCallData<EclipseResultRequest, DoubleResult>("results"));
    process(new RiaGrpcServerCallData<Case, Int32Message>("numberOfTimeSteps"));
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::processOneRequest()
{
    std::lock_guard<std::mutex> requestLock(m_requestMutex);
    if (!m_receivedRequests.empty())
    {
        RiaGrpcServerCallMethod* method = m_receivedRequests.front();
        m_receivedRequests.pop_front();
        process(method);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::waitForNextRequest()
{
    void* tag;
    bool  ok = false;

    while (m_completionQueue->Next(&tag, &ok))
    {
        if (!ok)
        {
            return;
        }
        std::lock_guard<std::mutex> requestLock(m_requestMutex);
        RiaGrpcServerCallMethod* method = static_cast<RiaGrpcServerCallMethod*>(tag);
        m_receivedRequests.push_back(method);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::process(RiaGrpcServerCallMethod* method)
{
    RiaGrpcServerCallData<Case, Vec3i>* dimCd = dynamic_cast<RiaGrpcServerCallData<Case, Vec3i>*>(method);
    RiaGrpcServerCallData<EclipseResultRequest, DoubleResult>* resultsCd = dynamic_cast<RiaGrpcServerCallData<EclipseResultRequest, DoubleResult>*>(method);
    RiaGrpcServerCallData<Case, Int32Message>* tsCd = dynamic_cast<RiaGrpcServerCallData<Case, Int32Message>*>(method);

    if (method->callStatus() == RiaGrpcServerCallMethod::CREATE)
    {
        method->callStatus() = RiaGrpcServerCallMethod::PROCESS;
        if (dimCd)
        {
            m_service.Requestdimensions(&(dimCd->context()),
                                        &(dimCd->request()),
                                        &(dimCd->responder()),
                                        m_completionQueue.get(),
                                        m_completionQueue.get(),
                                        dimCd);                                       
        }
        else if (resultsCd)
        {
            m_service.Requestresults(&(resultsCd->context()),
                                     &(resultsCd->request()),
                                     &(resultsCd->responder()),
                                     m_completionQueue.get(),
                                     m_completionQueue.get(),
                                     resultsCd);
        }
        else if (tsCd)
        {
            m_service.RequestnumberOfTimeSteps(&(tsCd->context()),
                                               &(tsCd->request()),
                                               &(tsCd->responder()),
                                               m_completionQueue.get(),
                                               m_completionQueue.get(),
                                               tsCd);
        }
    }
    else if (method->callStatus() == RiaGrpcServerCallMethod::PROCESS)
    {
        method->callStatus() = RiaGrpcServerCallMethod::FINISH;
        if (dimCd)
        {
            process(new RiaGrpcServerCallData<Case, Vec3i>("dimensions"));
            Status status = m_service.dimensions(&(dimCd->context()), &(dimCd->request()), &(dimCd->reply()));
            dimCd->responder().Finish(dimCd->reply(), status, dimCd);
            
        }
        else if (resultsCd)
        {
            process(new RiaGrpcServerCallData<EclipseResultRequest, DoubleResult>("results"));
            Status status = m_service.results(&(resultsCd->context()), &(resultsCd->request()), &(resultsCd->reply()));
            resultsCd->responder().Finish(resultsCd->reply(), status, resultsCd);
        }
        else if (tsCd)
        {
            process(new RiaGrpcServerCallData<Case, Int32Message>("numberOfTimeSteps"));
            Status status = m_service.numberOfTimeSteps(&(tsCd->context()), &(tsCd->request()), &(tsCd->reply()));
            tsCd->responder().Finish(tsCd->reply(), status, tsCd);
        }
    }
    else
    {
        delete method;
    }
}
