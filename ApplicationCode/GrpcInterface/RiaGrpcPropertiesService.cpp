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
#include "RiaGrpcCaseService.h"

#include "RigActiveCellInfo.h"
#include "RigActiveCellsResultAccessor.h"
#include "RigAllGridCellsResultAccessor.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigEclipseResultInfo.h"
#include "RigMainGrid.h"
#include "RigResultAccessor.h"
#include "RigResultAccessorFactory.h"
#include "RigResultModifier.h"
#include "RigResultModifierFactory.h"

#include "Rim3dView.h"
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
    RiaCellResultsStateHandler(bool clientStreamer = false)
        : m_request(nullptr)
        , m_currentCellIdx(0u)
        , m_clientStreamer(clientStreamer)
    {
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status init(const PropertyRequest* request)
    {
        int caseId    = request->case_request().id();
        m_eclipseCase = dynamic_cast<RimEclipseCase*>(RiaGrpcServiceInterface::findCase(caseId));

        if (m_eclipseCase)
        {
            auto                    porosityModel = static_cast<RiaDefines::PorosityModelType>(request->porosity_model());
            auto                    caseData      = m_eclipseCase->eclipseCaseData();
            auto                    resultData    = caseData->results(porosityModel);
            auto                    resultType    = static_cast<RiaDefines::ResultCatType>(request->property_type());
            size_t                  timeStep      = static_cast<size_t>(request->time_step());
            RigEclipseResultAddress resAddr(resultType, QString::fromStdString(request->property_name()));

            if (resultData->ensureKnownResultLoaded(resAddr))
            {
                if (timeStep < resultData->timeStepCount(resAddr))
                {
                    initResultAccess(caseData, request->grid_index(), porosityModel, timeStep, resAddr);
                    return grpc::Status::OK;
                }
                return grpc::Status(grpc::NOT_FOUND, "No such time step");
            }
            else if (m_clientStreamer)
            {
                resultData->createResultEntry(resAddr, true);
                RigEclipseResultAddress addrToMaxTimeStepCountResult;

                size_t timeStepCount = resultData->maxTimeStepCount(&addrToMaxTimeStepCountResult);
                const std::vector<RigEclipseTimeStepInfo> timeStepInfos =
                    resultData->timeStepInfos(addrToMaxTimeStepCountResult);
                resultData->setTimeStepInfos(resAddr, timeStepInfos);
                auto scalarResultFrames = resultData->modifiableCellScalarResultTimesteps(resAddr);
                scalarResultFrames.resize(timeStepCount);
                if (timeStep < resultData->timeStepCount(resAddr))
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
    /// Client streamers need to be initialised with the encapsulated parameters
    //--------------------------------------------------------------------------------------------------
    Status init(const PropertyInputChunk* chunk)
    {
        if (chunk->has_params())
        {
            return init(&(chunk->params()));
        }
        return grpc::Status(grpc::INVALID_ARGUMENT, "Need to have PropertyRequest parameters in first message");
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status assignStreamReply(PropertyChunk* reply)
    {
        const size_t packageSize  = RiaGrpcServiceInterface::numberOfMessagesForByteCount(sizeof(rips::PropertyChunk));
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

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    Status receiveStreamRequest(const PropertyInputChunk* request)
    {
        if (request->has_values())
        {
            auto values = request->values().values();
            for (int i = 0; i < values.size() && m_currentCellIdx < m_cellCount; ++i, ++m_currentCellIdx)
            {
                setCellResult(m_currentCellIdx, values[i]);
            }
            if (m_currentCellIdx >= m_cellCount - 1)
            {
                return grpc::Status(grpc::OUT_OF_RANGE, "All values have been written");
            }

            return Status::OK;
        }
        return grpc::Status(grpc::OUT_OF_RANGE, "No messages to write");
    }

    //--------------------------------------------------------------------------------------------------
    ///
    //--------------------------------------------------------------------------------------------------
    void finish()
    {
        for (Rim3dView* view : m_eclipseCase->views())
        {
            view->setCurrentTimeStepAndUpdate(view->currentTimeStep());
            view->createDisplayModelAndRedraw();
        }
    }

protected:
    virtual void   initResultAccess(RigEclipseCaseData*           caseData,
                                    size_t                        gridIndex,
                                    RiaDefines::PorosityModelType porosityModel,
                                    size_t                        timeStepIndex,
                                    RigEclipseResultAddress       resVarAddr) = 0;
    virtual double cellResult(size_t currentCellIndex) const                  = 0;
    virtual void   setCellResult(size_t currentCellIndex, double value)       = 0;

protected:
    const rips::PropertyRequest*  m_request;
    RimEclipseCase*             m_eclipseCase;
    size_t                      m_currentCellIdx;
    size_t                      m_cellCount;
    bool                        m_clientStreamer;
};

class RiaActiveCellResultsStateHandler : public RiaCellResultsStateHandler
{
public:
    RiaActiveCellResultsStateHandler(bool clientStreamer = false)
        : RiaCellResultsStateHandler(clientStreamer)
    {}
protected:
    void initResultAccess(RigEclipseCaseData*           caseData,
                          size_t                        gridIndex,
                          RiaDefines::PorosityModelType porosityModel,
                          size_t                        timeStepIndex,
                          RigEclipseResultAddress       resVarAddr) override
    {
        auto activeCellInfo = caseData->activeCellInfo(porosityModel);
        m_resultValues      = &(caseData->results(porosityModel)->modifiableCellScalarResult(resVarAddr, timeStepIndex));
        if (m_resultValues->empty())
        {
            m_resultValues->resize(activeCellInfo->reservoirCellResultCount());
        }
        m_cellCount         = activeCellInfo->reservoirActiveCellCount();
    }

    double cellResult(size_t currentCellIndex) const override
    {
        return (*m_resultValues)[currentCellIndex];
    }

    void setCellResult(size_t currentCellIndex, double value) override
    {
        (*m_resultValues)[currentCellIndex] = value;
    }

private:
    std::vector<double>* m_resultValues;
};

class RiaGridCellResultsStateHandler : public RiaCellResultsStateHandler
{
public:
    RiaGridCellResultsStateHandler(bool clientStreamer = false)
        : RiaCellResultsStateHandler(clientStreamer)
    {}

protected:
    void initResultAccess(RigEclipseCaseData*           caseData,
                          size_t                        gridIndex,
                          RiaDefines::PorosityModelType porosityModel,
                          size_t                        timeStepIndex,
                          RigEclipseResultAddress       resVarAddr) override
    {
        m_resultAccessor = RigResultAccessorFactory::createFromResultAddress(caseData, gridIndex, porosityModel, timeStepIndex, resVarAddr);
        m_resultModifier = RigResultModifierFactory::createResultModifier(caseData, gridIndex, porosityModel, timeStepIndex, resVarAddr);
        m_cellCount      = caseData->grid(gridIndex)->cellCount();
    }

    double cellResult(size_t currentCellIndex) const override
    {
        return m_resultAccessor->cellScalar(currentCellIndex);
    }

    void setCellResult(size_t currentCellIndex, double value) override
    {
        return m_resultModifier->setCellScalar(currentCellIndex, value);
    }

private:
    cvf::ref<RigResultAccessor> m_resultAccessor;
    cvf::ref<RigResultModifier> m_resultModifier;
};

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetAvailableProperties(grpc::ServerContext*              context,
                                                              const AvailablePropertiesRequest* request,
                                                              AvailableProperties*              reply)
{
    int             caseId      = request->case_request().id();
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
grpc::Status RiaGrpcPropertiesService::GetActiveCellProperty(grpc::ServerContext*             context,
                                                            const PropertyRequest*            request,
                                                            PropertyChunk*                    reply,
                                                            RiaActiveCellResultsStateHandler* stateHandler)
{
    return stateHandler->assignStreamReply(reply);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::GetGridProperty(grpc::ServerContext*           context,
                                                      const rips::PropertyRequest*    request,
                                                      rips::PropertyChunk*            reply,
                                                      RiaGridCellResultsStateHandler* stateHandler)
{
    return stateHandler->assignStreamReply(reply);
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::SetActiveCellProperty(grpc::ServerContext*             context,
                                                            const rips::PropertyInputChunk*   request,
                                                            rips::Empty*                      reply,
                                                            RiaActiveCellResultsStateHandler* stateHandler)
{
    return stateHandler->receiveStreamRequest(request);
}


//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcPropertiesService::SetGridProperty(grpc::ServerContext*           context,
                                                      const rips::PropertyInputChunk* request,
                                                      rips::Empty*                    reply,
                                                      RiaGridCellResultsStateHandler* stateHandler)
{
    return stateHandler->receiveStreamRequest(request);
}
//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcPropertiesService::createCallbacks()
{
    typedef RiaGrpcPropertiesService Self;

    return {new RiaGrpcUnaryCallback<Self, AvailablePropertiesRequest, AvailableProperties>(
                this, &Self::GetAvailableProperties, &Self::RequestGetAvailableProperties),
            new RiaGrpcServerStreamCallback<Self, PropertyRequest, PropertyChunk, RiaActiveCellResultsStateHandler>(
                this, &Self::GetActiveCellProperty, &Self::RequestGetActiveCellProperty, new RiaActiveCellResultsStateHandler),
            new RiaGrpcServerStreamCallback<Self, PropertyRequest, PropertyChunk, RiaGridCellResultsStateHandler>(
                this, &Self::GetGridProperty, &Self::RequestGetGridProperty, new RiaGridCellResultsStateHandler),
            new RiaGrpcClientStreamCallback<Self, PropertyInputChunk, Empty, RiaActiveCellResultsStateHandler>(
                this, &Self::SetActiveCellProperty, &Self::RequestSetActiveCellProperty, new RiaActiveCellResultsStateHandler(true)),
            new RiaGrpcClientStreamCallback<Self, PropertyInputChunk, Empty, RiaGridCellResultsStateHandler>(
                this, &Self::SetGridProperty, &Self::RequestSetGridProperty, new RiaGridCellResultsStateHandler(true))
    };
}


static bool RiaGrpcPropertiesService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcPropertiesService>(typeid(RiaGrpcPropertiesService).hash_code());