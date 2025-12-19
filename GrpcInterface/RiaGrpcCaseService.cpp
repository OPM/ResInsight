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
#include "RiaGrpcCaseService.h"

#include "RiaGrpcActiveCellInfoStateHandler.h"
#include "RiaGrpcCallbacks.h"
#include "RiaGrpcHelper.h"
#include "RiaGrpcSelectedCellsStateHandler.h"
#include "RiaSocketTools.h"

#include "RigActiveCellInfo.h"
#include "RigCaseCellResultsData.h"
#include "RigEclipseCaseData.h"
#include "RigEclipseResultAddress.h"
#include "RigMainGrid.h"

#include "RimEclipseCase.h"
#include "RimEclipseResultDefinition.h"

#include "Riu3dSelectionManager.h"

#include <array>

using namespace rips;

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetGridCount( grpc::ServerContext*     context,
                                               const rips::CaseRequest* request,
                                               rips::GridCount*         reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->id() );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( eclipseCase )
    {
        int gridCount = (int)eclipseCase->mainGrid()->gridCount();
        reply->set_count( gridCount );
        return Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellCount( grpc::ServerContext*         context,
                                               const rips::CellInfoRequest* request,
                                               rips::CellCount*             reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->case_request().id() );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( eclipseCase )
    {
        auto               porosityModel  = RiaDefines::PorosityModelType( request->porosity_model() );
        RigActiveCellInfo* activeCellInfo = eclipseCase->eclipseCaseData()->activeCellInfo( porosityModel );
        reply->set_active_cell_count( (int)activeCellInfo->reservoirActiveCellCount() );
        reply->set_reservoir_cell_count( (int)activeCellInfo->reservoirCellCount() );
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetTimeSteps( grpc::ServerContext*     context,
                                               const rips::CaseRequest* request,
                                               rips::TimeStepDates*     reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->id() );

    if ( rimCase )
    {
        std::vector<QDateTime> timeStepDates = rimCase->timeStepDates();
        for ( QDateTime dateTime : timeStepDates )
        {
            rips::TimeStepDate* date = reply->add_dates();
            date->set_year( dateTime.date().year() );
            date->set_month( dateTime.date().month() );
            date->set_day( dateTime.date().day() );
            date->set_hour( dateTime.time().hour() );
            date->set_minute( dateTime.time().minute() );
            date->set_second( dateTime.time().second() );
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetDaysSinceStart( grpc::ServerContext*     context,
                                                    const rips::CaseRequest* request,
                                                    rips::DaysSinceStart*    reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->id() );

    RimEclipseCase* eclipseCase = dynamic_cast<RimEclipseCase*>( rimCase );
    if ( eclipseCase )
    {
        RigEclipseResultAddress addrToMaxTimeStepCountResult;
        if ( eclipseCase && eclipseCase->eclipseCaseData() )
        {
            eclipseCase->eclipseCaseData()
                ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                ->maxTimeStepCount( &addrToMaxTimeStepCountResult );
            if ( !addrToMaxTimeStepCountResult.isValid() )
            {
                return grpc::Status( grpc::NOT_FOUND, "Invalid result. No time steps found." );
            }
        }

        std::vector<double> daysSinceSimulationStart = eclipseCase->eclipseCaseData()
                                                           ->results( RiaDefines::PorosityModelType::MATRIX_MODEL )
                                                           ->daysSinceSimulationStart( addrToMaxTimeStepCountResult );

        for ( auto days : daysSinceSimulationStart )
        {
            reply->add_day_decimals( days );
        }
        return grpc::Status::OK;
    }
    return grpc::Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcCaseService::GetCaseInfo( grpc::ServerContext* context, const rips::CaseRequest* request, rips::CaseInfo* reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->id() );
    if ( rimCase )
    {
        qint64  caseId      = rimCase->caseId();
        qint64  caseGroupId = -1;
        QString caseName, caseType;
        RiaSocketTools::getCaseInfoFromCase( rimCase, caseId, caseName, caseType, caseGroupId );

        reply->set_id( caseId );
        reply->set_group_id( caseGroupId );
        reply->set_name( caseName.toStdString() );
        reply->set_type( caseType.toStdString() );
        return Status::OK;
    }
    return Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetPdmObject( grpc::ServerContext*     context,
                                               const rips::CaseRequest* request,
                                               rips::PdmObject*         reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->id() );
    if ( rimCase )
    {
        copyPdmObjectFromCafToRips( rimCase, reply );
    }
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellInfoForActiveCells( grpc::ServerContext*               context,
                                                            const rips::CellInfoRequest*       request,
                                                            rips::CellInfoArray*               reply,
                                                            RiaGrpcActiveCellInfoStateHandler* stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellCenterForActiveCells( grpc::ServerContext*               context,
                                                              const rips::CellInfoRequest*       request,
                                                              rips::CellCenters*                 reply,
                                                              RiaGrpcActiveCellInfoStateHandler* stateHandler )
{
    return stateHandler->assignCellCentersReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCellCornersForActiveCells( grpc::ServerContext*               context,
                                                               const rips::CellInfoRequest*       request,
                                                               rips::CellCornersArray*            reply,
                                                               RiaGrpcActiveCellInfoStateHandler* stateHandler )
{
    return stateHandler->assignCellCornersReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetSelectedCells( grpc::ServerContext*              context,
                                                   const rips::CaseRequest*          request,
                                                   rips::SelectedCells*              reply,
                                                   RiaGrpcSelectedCellsStateHandler* stateHandler )
{
    return stateHandler->assignReply( reply );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetReservoirBoundingBox( grpc::ServerContext*     context,
                                                          const rips::CaseRequest* request,
                                                          rips::BoundingBox*       reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->id() );
    if ( rimCase )
    {
        cvf::BoundingBox boundingBox = rimCase->reservoirBoundingBox();
        reply->set_min_x( boundingBox.min().x() );
        reply->set_min_y( boundingBox.min().y() );
        reply->set_min_z( boundingBox.min().z() );
        reply->set_max_x( boundingBox.max().x() );
        reply->set_max_y( boundingBox.max().y() );
        reply->set_max_z( boundingBox.max().z() );
        return Status::OK;
    }
    return Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetCoarseningInfoArray( grpc::ServerContext*       context,
                                                         const rips::CaseRequest*   request,
                                                         rips::CoarseningInfoArray* reply )
{
    RimEclipseCase* rimCase = dynamic_cast<RimEclipseCase*>( RiaGrpcHelper::findCase( request->id() ) );
    if ( rimCase && rimCase->eclipseCaseData() && rimCase->eclipseCaseData()->mainGrid() )
    {
        for ( size_t gridIdx = 0; gridIdx < rimCase->eclipseCaseData()->gridCount(); gridIdx++ )
        {
            RigGridBase* grid = rimCase->eclipseCaseData()->grid( gridIdx );

            size_t localCoarseningBoxCount = grid->coarseningBoxCount();
            for ( size_t boxIdx = 0; boxIdx < localCoarseningBoxCount; boxIdx++ )
            {
                size_t i1, i2, j1, j2, k1, k2;
                grid->coarseningBox( boxIdx, &i1, &i2, &j1, &j2, &k1, &k2 );

                rips::CoarseningInfo* coarseningInfo = reply->add_data();

                rips::Vec3i* min = new rips::Vec3i;
                min->set_i( (int)i1 );
                min->set_j( (int)j1 );
                min->set_k( (int)k1 );
                coarseningInfo->set_allocated_min( min );

                rips::Vec3i* max = new rips::Vec3i;
                max->set_i( (int)i2 );
                max->set_j( (int)j2 );
                max->set_k( (int)k2 );
                coarseningInfo->set_allocated_max( max );
            }
        }
        return Status::OK;
    }
    return Status( grpc::NOT_FOUND, "Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcCaseService::GetDistanceToClosestFault( grpc::ServerContext*             context,
                                                            const rips::ClosestFaultRequest* request,
                                                            rips::ClosestFault*              reply )
{
    RimCase* rimCase = RiaGrpcHelper::findCase( request->case_request().id() );
    if ( auto eCase = dynamic_cast<RimEclipseCase*>( rimCase ) )
    {
        cvf::Vec3d point( request->point() );
        point.z() = -point.z(); // Convert to internal coordinate system

        auto [faultName, distance, faceType] = eCase->mainGrid()->minimumDistanceFaultToPoint( point );

        reply->set_distance( distance );
        reply->set_face_name( RiaGrpcHelper::faceTypeToString( faceType ) );
        reply->set_fault_name( faultName.toStdString() );

        return grpc::Status::OK;
    }
    return Status( grpc::NOT_FOUND, "Eclipse Case not found" );
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcCaseService::createCallbacks()
{
    using Self = RiaGrpcCaseService;

    return { new RiaGrpcUnaryCallback<Self, CaseRequest, GridCount>( this, &Self::GetGridCount, &Self::RequestGetGridCount ),
             new RiaGrpcUnaryCallback<Self, CellInfoRequest, CellCount>( this, &Self::GetCellCount, &Self::RequestGetCellCount ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, TimeStepDates>( this, &Self::GetTimeSteps, &Self::RequestGetTimeSteps ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, DaysSinceStart>( this,
                                                                          &Self::GetDaysSinceStart,
                                                                          &Self::RequestGetDaysSinceStart ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, CaseInfo>( this, &Self::GetCaseInfo, &Self::RequestGetCaseInfo ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, PdmObject>( this, &Self::GetPdmObject, &Self::RequestGetPdmObject ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CellInfoRequest,
                                                     CellInfoArray,
                                                     RiaGrpcActiveCellInfoStateHandler>( this,
                                                                                         &Self::GetCellInfoForActiveCells,
                                                                                         &Self::RequestGetCellInfoForActiveCells,
                                                                                         new RiaGrpcActiveCellInfoStateHandler ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CellInfoRequest,
                                                     CellCenters,
                                                     RiaGrpcActiveCellInfoStateHandler>( this,
                                                                                         &Self::GetCellCenterForActiveCells,
                                                                                         &Self::RequestGetCellCenterForActiveCells,
                                                                                         new RiaGrpcActiveCellInfoStateHandler ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CellInfoRequest,
                                                     CellCornersArray,
                                                     RiaGrpcActiveCellInfoStateHandler>( this,
                                                                                         &Self::GetCellCornersForActiveCells,
                                                                                         &Self::RequestGetCellCornersForActiveCells,
                                                                                         new RiaGrpcActiveCellInfoStateHandler ),
             new RiaGrpcServerToClientStreamCallback<Self,
                                                     CaseRequest,
                                                     SelectedCells,
                                                     RiaGrpcSelectedCellsStateHandler>( this,
                                                                                        &Self::GetSelectedCells,
                                                                                        &Self::RequestGetSelectedCells,
                                                                                        new RiaGrpcSelectedCellsStateHandler ),
             new RiaGrpcUnaryCallback<Self, CaseRequest, BoundingBox>( this,
                                                                       &Self::GetReservoirBoundingBox,
                                                                       &Self::RequestGetReservoirBoundingBox ),

             new RiaGrpcUnaryCallback<Self, CaseRequest, CoarseningInfoArray>( this,
                                                                               &Self::GetCoarseningInfoArray,
                                                                               &Self::RequestGetCoarseningInfoArray ),

             new RiaGrpcUnaryCallback<Self, ClosestFaultRequest, ClosestFault>( this,
                                                                                &Self::GetDistanceToClosestFault,
                                                                                &Self::RequestGetDistanceToClosestFault ) };
}

static bool RiaGrpcCaseService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcCaseService>( typeid( RiaGrpcCaseService ).hash_code() );
