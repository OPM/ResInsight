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
#pragma once

#include <grpcpp/grpcpp.h>

#include <vector>

class RiaAbstractGrpcCallback;
class RimCase;

//==================================================================================================
//
// gRPC-service interface which all gRPC-services has to implement
//
//==================================================================================================
class RiaGrpcServiceInterface
{
public:
    virtual std::vector<RiaAbstractGrpcCallback*> createCallbacks() = 0;
    virtual ~RiaGrpcServiceInterface() = default;
    static RimCase* findCase(int caseId);
    static size_t numberOfMessagesForByteCount(size_t messageSize, size_t byteCount = 64 * 1024u);
};

#include "cafFactory.h"
typedef caf::Factory<RiaGrpcServiceInterface, size_t> RiaGrpcServiceFactory;

