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
#include "RiaGrpcAppService.h"

#include "RiaApplication.h"
#include "RiaGrpcCallbacks.h"
#include "RiaGrpcServer.h"
#include "RiaVersionInfo.h"

#include <QApplication>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcAppService::GetVersion( grpc::ServerContext* context, const rips::Empty* request, rips::Version* reply )
{
    reply->set_major_version( QString( RESINSIGHT_MAJOR_VERSION ).toInt() );
    reply->set_minor_version( QString( RESINSIGHT_MINOR_VERSION ).toInt() );
    reply->set_patch_version( QString( RESINSIGHT_PATCH_VERSION ).toInt() );
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcAppService::Exit( grpc::ServerContext* context, const rips::Empty* request, rips::Empty* reply )
{
    RiaGrpcServer::setReceivedExitRequest();
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status
    RiaGrpcAppService::GetRuntimeInfo( grpc::ServerContext* context, const rips::Empty* request, rips::RuntimeInfo* reply )
{
    rips::ApplicationTypeEnum appType = rips::CONSOLE_APPLICATION;
    if ( dynamic_cast<QApplication*>(RiaApplication::instance()))
    {
        appType = rips::GUI_APPLICATION;
    }
    reply->set_app_type( appType );

    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaGrpcCallbackInterface*> RiaGrpcAppService::createCallbacks()
{
    typedef RiaGrpcAppService Self;
    return { new RiaGrpcUnaryCallback<Self, rips::Empty, rips::Version>( this, &Self::GetVersion, &Self::RequestGetVersion ),
             new RiaGrpcUnaryCallback<Self, rips::Empty, rips::Empty>( this, &Self::Exit, &Self::RequestExit ),
             new RiaGrpcUnaryCallback<Self, rips::Empty, rips::RuntimeInfo>( this,
                                                                             &Self::GetRuntimeInfo,
                                                                             &Self::RequestGetRuntimeInfo ) };
}

static bool RiaGrpcAppInfoService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcAppService>( typeid( RiaGrpcAppService ).hash_code() );
