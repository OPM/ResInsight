/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2025     Equinor ASA
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

#include "Case.grpc.pb.h"

#include "RiaGrpcServiceInterface.h"

#include <vector>

namespace rips
{
class CaseRequest;
} // namespace rips

class RimEclipseCase;
class RiuEclipseSelectionItem;

//==================================================================================================
//
// State handler for streaming of selected cells
//
//==================================================================================================
class RiaGrpcSelectedCellsStateHandler
{
    using Status = grpc::Status;

public:
    RiaGrpcSelectedCellsStateHandler();

    Status init( const rips::CaseRequest* request );
    Status assignReply( rips::SelectedCells* reply );
    void   assignSelectedCell( rips::SelectedCell* cell, const RiuEclipseSelectionItem* item );
    Status assignNextSelectedCell( rips::SelectedCell* cell, const std::vector<RiuEclipseSelectionItem*>& items );

protected:
    const rips::CaseRequest* m_request;
    RimEclipseCase*          m_eclipseCase;
    size_t                   m_currentItem;
};
