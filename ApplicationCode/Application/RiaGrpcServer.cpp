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

#include "grpc/impl/codegen/gpr_types.h"

#include <QTcpServer>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimEclipseCase* RiaGrpcGridServiceImpl::getCase(int caseId) const
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
Status RiaGrpcGridServiceImpl::dimensions(ServerContext* context, const Case* request, Vec3i* reply)
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
Status RiaGrpcGridServiceImpl::results(ServerContext* context, const EclipseResultRequest* request, DoubleResult* result)
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
Status RiaGrpcGridServiceImpl::numberOfTimeSteps(ServerContext* context, const Case* request, Int32Message* reply)
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
Status RiaGrpcProjectInfoServiceImpl::GetCurrentCase(ServerContext* context, const Empty* request, Case* reply)
{
    reply->set_id(1);
    return Status::OK;
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaGrpcServer::~RiaGrpcServer()
{
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
    std::thread(&RiaGrpcServer::waitForNextRequest, this).detach();
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::initialize()
{
    int port = 50051;
    {
        QTcpServer serverTest;
        while (!serverTest.listen(QHostAddress::LocalHost, port))
        {
            port++;
        }
    }

    QString serverAddress = QString("localhost:%1").arg(port);

    ServerBuilder builder;
    builder.AddListeningPort(serverAddress.toStdString(), grpc::InsecureServerCredentials());

    builder.RegisterService(&m_service);
    builder.RegisterService(&m_projectService);
    m_completionQueue = builder.AddCompletionQueue();
    m_server = builder.BuildAndStart();

    CVF_ASSERT(m_server);
    RiaLogging::info(QString("Server listening on %1").arg(serverAddress));
    // Spawn new CallData instances to serve new clients.
    process(new RiaGrpcServerCallData<RiaGrpcGridServiceImpl, Case, Vec3i>(&m_service, m_completionQueue.get(), "dimensions", &RiaGrpcGridServiceImpl::dimensions, &RiaGrpcGridServiceImpl::Requestdimensions));
    process(new RiaGrpcServerCallData<RiaGrpcGridServiceImpl, EclipseResultRequest, DoubleResult>(&m_service, m_completionQueue.get(), "results", &RiaGrpcGridServiceImpl::results, &RiaGrpcGridServiceImpl::Requestresults));
    process(new RiaGrpcServerCallData<RiaGrpcGridServiceImpl, Case, Int32Message>(&m_service, m_completionQueue.get(), "numberOfTimeSteps", &RiaGrpcGridServiceImpl::numberOfTimeSteps, &RiaGrpcGridServiceImpl::RequestnumberOfTimeSteps));

    process(new RiaGrpcServerCallData<RiaGrpcProjectInfoServiceImpl, Empty, Case>(&m_projectService, m_completionQueue.get(), "GetCurrentCase", &RiaGrpcProjectInfoServiceImpl::GetCurrentCase, &RiaGrpcProjectInfoServiceImpl::RequestGetCurrentCase));
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
void RiaGrpcServer::quit()
{
    m_server->Shutdown();
    m_completionQueue->Shutdown();
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
        RiaGrpcServerCallMethod* method = static_cast<RiaGrpcServerCallMethod*>(tag);
        if (!ok)
        {
            method->callStatus() = RiaGrpcServerCallMethod::FINISH;
            process(method);
        }
        std::lock_guard<std::mutex> requestLock(m_requestMutex);
        m_receivedRequests.push_back(method);
    }
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
void RiaGrpcServer::process(RiaGrpcServerCallMethod* method)
{
    if (method->callStatus() == RiaGrpcServerCallMethod::CREATE)
    {
        method->callStatus() = RiaGrpcServerCallMethod::PROCESS;
        method->createRequest();
    }
    else if (method->callStatus() == RiaGrpcServerCallMethod::PROCESS)
    {
        method->callStatus() = RiaGrpcServerCallMethod::FINISH;
        method->processRequest();
        process(method->clone());
    }
    else
    {
        delete method;
    }
}
