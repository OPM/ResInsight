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
#include "RiaGrpcServiceInterface.h"

#include "RiaApplication.h"
#include "RimProject.h"
#include "RimCase.h"

#include <grpcpp/grpcpp.h>

//--------------------------------------------------------------------------------------------------
///
//--------------------------------------------------------------------------------------------------
RimCase* RiaGrpcServiceInterface::findCase(int caseId)
{
    std::vector<RimCase*> cases;
    RiaApplication::instance()->project()->allCases(cases);

    for (RimCase* rimCase : cases)
    {
        if (caseId == rimCase->caseId())
        {
            return rimCase;
        }
    }
    return nullptr;
}

//--------------------------------------------------------------------------------------------------
/// Find the number of messages that will fit in the given bytes.
/// The default argument is meant to be a sensible size for GRPC.
//--------------------------------------------------------------------------------------------------
size_t RiaGrpcServiceInterface::numberOfMessagesForByteCount(size_t messageSize,
                                                             size_t numBytesWantedInPackage /*= 64 * 1024u*/)
{
    size_t messageCount = numBytesWantedInPackage / messageSize;
    return messageCount;
}

