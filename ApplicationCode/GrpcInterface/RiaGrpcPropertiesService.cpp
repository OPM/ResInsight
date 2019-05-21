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
#include "RiaGrpcPropertiesService.h"

#include "RiaGrpcGridInfoService.h"
#include "RiaGrpcCallbacks.h"

#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"

#include "RimEclipseCase.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RiaActiveCellResultsStateHandler::RiaActiveCellResultsStateHandler()
    : m_request(nullptr)
    , m_resultValues(nullptr)
    , m_currentCellIdx(0u)
{
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellResultsStateHandler::init(const ResultRequest* request)
{
    int caseId = request->request_case().id();
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(RiaGrpcServiceInterface::findCase(caseId));
    if (eclipseCase)
    {
        auto porosityModel = static_cast<RiaDefines::PorosityModelType>(request->porosity_model());
        auto resultData    = eclipseCase->eclipseCaseData()->results(porosityModel);
        auto resultType    = static_cast<RiaDefines::ResultCatType>(request->property_type());
        size_t timeStep    = static_cast<size_t>(request->time_step());
        RigEclipseResultAddress resAddr(resultType, QString::fromStdString(request->property_name()));

        if (resultData->hasResultEntry(resAddr))
        {
            if (timeStep < (int)resultData->timeStepCount(resAddr))
            {
                m_resultValues = &resultData->cellScalarResults(resAddr, timeStep);
                return grpc::Status::OK;
            }
            return grpc::Status(grpc::NOT_FOUND, "No such time step");
        }
        return grpc::Status(grpc::NOT_FOUND, "No such result");
    }
    return grpc::Status(grpc::NOT_FOUND, "Couldn't find an Eclipse case matching the case Id");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaActiveCellResultsStateHandler::assignReply(ResultReplyArray* reply)
{
    if (m_resultValues)
    {
        const size_t packageSize = RiaGrpcServiceInterface::numberOfMessagesForByteCount(sizeof(rips::ResultReplyArray));
        size_t       packageIndex = 0u;
        reply->mutable_values()->Reserve((int)packageSize);
        for (; packageIndex < packageSize && m_currentCellIdx < m_resultValues->size(); ++packageIndex, ++m_currentCellIdx)
        {
            reply->add_values(m_resultValues->at(m_currentCellIdx));
        }
        if (packageIndex > 0u)
        {
            return grpc::Status::OK;
        }
        return grpc::Status(grpc::OUT_OF_RANGE, "We've reached the end. This is not an error but means transmission is finished");
    }
    return grpc::Status(grpc::NOT_FOUND, "No result values found");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetAvailableProperties(grpc::ServerContext*     context,
                                                              const PropertiesRequest* request,
                                                              AvailableProperties*     reply)
{
    int             caseId      = request->request_case().id();
    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(RiaGrpcServiceInterface::findCase(caseId));
    if (eclipseCase)
    {
        auto        porosityModel = static_cast<RiaDefines::PorosityModelType>(request->porosity_model());
        auto        resultData    = eclipseCase->eclipseCaseData()->results(porosityModel);
        auto        resultType    = static_cast<RiaDefines::ResultCatType>(request->property_type());
        QStringList resultNames   = resultData->resultNames(resultType);
        if (!resultNames.empty())
        {
            for (QString resultName : resultNames)
            {
                reply->add_property_names(resultName.toStdString());
            }
            return grpc::Status::OK;
        }
        return grpc::Status(grpc::NOT_FOUND, "Could not find any results matching result type");
    }
    return grpc::Status(grpc::NOT_FOUND, "No such case");
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetActiveCellResults(grpc::ServerContext*               context,
                                                             const ResultRequest*             request,
                                                             ResultReplyArray*                reply,
                                                             RiaActiveCellResultsStateHandler* stateHandler)
{
    return stateHandler->assignReply(reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaAbstractGrpcCallback*> RiaGrpcPropertiesService::createCallbacks()
{
    typedef RiaGrpcPropertiesService Self;

    return { new RiaGrpcCallback<Self, PropertiesRequest, AvailableProperties>(this, &Self::GetAvailableProperties, &Self::RequestGetAvailableProperties),
             new RiaGrpcStreamCallback<Self, ResultRequest, ResultReplyArray, RiaActiveCellResultsStateHandler>(
                 this, &Self::GetActiveCellResults, &Self::RequestGetActiveCellResults, new RiaActiveCellResultsStateHandler)
    };
}

static bool RiaGrpcPropertiesService_init = RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcPropertiesService>(typeid(RiaGrpcPropertiesService).hash_code());