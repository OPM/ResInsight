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

#include "RiaGrpcCallbacks.h"
#include "RiaGrpcGridInfoService.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigAllGridCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"

#include "RimEclipseCase.h"

using namespace rips;

//--------------------------------------------------------------------------------------------------
/// Abstract handler base class for streaming cell results to client
///
//--------------------------------------------------------------------------------------------------

class RiaCellResultsStateHandler
{
    typedef grpc::Status Status;

public:
    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    RiaCellResultsStateHandler()
        : m_request(nullptr)
        , m_currentCellIdx(0u)
    {
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status init(const ResultRequest* request)
    {
        int             caseId      = request->request_case().id();
        RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>(RiaGrpcServiceInterface::findCase(caseId));
        if (eclipseCase)
        {
            auto                    porosityModel = static_cast<RiaDefines::PorosityModelType>(request->porosity_model());
            auto                    caseData      = eclipseCase->eclipseCaseData();
            auto                    resultData    = caseData->results(porosityModel);
            auto                    resultType    = static_cast<RiaDefines::ResultCatType>(request->property_type());
            size_t                  timeStep      = static_cast<size_t>(request->time_step());
            RigEclipseResultAddress resAddr(resultType, QString::fromStdString(request->property_name()));

            if (resultData->ensureKnownResultLoaded(resAddr))
            {
                if (timeStep < (int)resultData->timeStepCount(resAddr))
                {
                    initResultAccess(caseData, request->grid_index(), porosityModel, timeStep, resAddr);
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
    Status assignReply(ResultReplyArray* reply)
    {
        const size_t packageSize  = RiaGrpcServiceInterface::numberOfMessagesForByteCount(sizeof(rips::ResultReplyArray));
        size_t       packageIndex = 0u;
        reply->mutable_values()->Reserve((int)packageSize);
        for (; packageIndex < packageSize && m_currentCellIdx < m_cellCount; ++packageIndex, ++m_currentCellIdx)
        {
            reply->add_values(cellResult(m_currentCellIdx));
        }
        if (packageIndex > 0u)
        {
            return grpc::Status::OK;
        }
        return grpc::Status(grpc::OUT_OF_RANGE,
                            "We've reached the end. This is not an error but means transmission is finished");
    }

protected:
    virtual void   initResultAccess(RigEclipseCaseData*           caseData,
                                    size_t                        gridIndex,
                                    RiaDefines::PorosityModelType porosityModel,
                                    size_t                        timeStepIndex,
                                    RigEclipseResultAddress       resVarAddr) = 0;
    virtual double cellResult(size_t currentCellIndex) const                  = 0;

protected:
    const rips::ResultRequest*  m_request;
    size_t                      m_currentCellIdx;
    size_t                      m_cellCount;
};

class RiaActiveCellResultsStateHandler : public RiaCellResultsStateHandler
{
protected:
    void initResultAccess(RigEclipseCaseData*           caseData,
                          size_t                        gridIndex,
                          RiaDefines::PorosityModelType porosityModel,
                          size_t                        timeStepIndex,
                          RigEclipseResultAddress       resVarAddr) override
    {
        auto activeCellInfo = caseData->activeCellInfo(porosityModel);
        m_resultValues      = &(caseData->results(porosityModel)->cellScalarResults(resVarAddr, timeStepIndex));
        m_cellCount         = activeCellInfo->reservoirActiveCellCount();
    }

    double cellResult(size_t currentCellIndex) const override
    {
        return (*m_resultValues)[currentCellIndex];
    }

private:
    const std::vector<double>* m_resultValues;
};

class RiaGridCellResultsStateHandler : public RiaCellResultsStateHandler
{
protected:
    void initResultAccess(RigEclipseCaseData*           caseData,
                          size_t                        gridIndex,
                          RiaDefines::PorosityModelType porosityModel,
                          size_t                        timeStepIndex,
                          RigEclipseResultAddress       resVarAddr) override
    {
        m_resultAccessor = RigResultAccessorFactory::createFromResultAddress(caseData, gridIndex, porosityModel, timeStepIndex, resVarAddr);
        m_cellCount      = caseData->grid(gridIndex)->cellCount();
    }

    double cellResult(size_t currentCellIndex) const override
    {
        return m_resultAccessor->cellScalar(currentCellIndex);
    }

private:
    cvf::ref<RigResultAccessor> m_resultAccessor;
};

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
grpc::Status RiaGrpcPropertiesService::GetActiveCellResults(grpc::ServerContext*              context,
                                                            const ResultRequest*              request,
                                                            ResultReplyArray*                 reply,
                                                            RiaActiveCellResultsStateHandler* stateHandler)
{
    return stateHandler->assignReply(reply);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetGridResults(grpc::ServerContext*            context,
                                                      const rips::ResultRequest*      request,
                                                      rips::ResultReplyArray*         reply,
                                                      RiaGridCellResultsStateHandler* stateHandler)
{
    return stateHandler->assignReply(reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaAbstractGrpcCallback*> RiaGrpcPropertiesService::createCallbacks()
{
    typedef RiaGrpcPropertiesService Self;

    return {new RiaGrpcCallback<Self, PropertiesRequest, AvailableProperties>(
                this, &Self::GetAvailableProperties, &Self::RequestGetAvailableProperties),
            new RiaGrpcStreamCallback<Self, ResultRequest, ResultReplyArray, RiaActiveCellResultsStateHandler>(
                this, &Self::GetActiveCellResults, &Self::RequestGetActiveCellResults, new RiaActiveCellResultsStateHandler),
            new RiaGrpcStreamCallback<Self, ResultRequest, ResultReplyArray, RiaGridCellResultsStateHandler>(
                this, &Self::GetGridResults, &Self::RequestGetGridResults, new RiaGridCellResultsStateHandler)    
    };
}

static bool RiaGrpcPropertiesService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcPropertiesService>(typeid(RiaGrpcPropertiesService).hash_code());