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
#include "RiaGrpcResInfoService.h"

#include "RiaVersionInfo.h"
#include "RiaGrpcCallbacks.h"

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
grpc::Status RiaGrpcResInfoService::GetVersion(grpc::ServerContext* context, const rips::Empty* request, rips::Version* reply)
{
    reply->set_major_version(RESINSIGHT_MAJOR_VERSION);
    reply->set_minor_version(RESINSIGHT_MINOR_VERSION);
    reply->set_patch_version(RESINSIGHT_PATCH_VERSION);
    return grpc::Status::OK;
}

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
std::vector<RiaAbstractGrpcCallback*> RiaGrpcResInfoService::createCallbacks()
{
    typedef RiaGrpcResInfoService Self;
    return { new RiaGrpcCallback<Self, rips::Empty, rips::Version>(this, &Self::GetVersion, &Self::RequestGetVersion) };
}

static bool RiaGrpcResInfoService_init =
    RiaGrpcServiceFactory::instance()->registerCreator<RiaGrpcResInfoService>(typeid(RiaGrpcResInfoService).hash_code());
