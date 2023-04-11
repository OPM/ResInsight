/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2023- Equinor ASA
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
/////////////////////////////////////////////////////////////////////////////////

#pragma once

#include "RiaDefines.h"

#include "RigWellResultPoint.h"

#include <QDateTime>

#include <vector>

//==================================================================================================
/// This class contains the well information for one timestep.
/// The main content is the vector of RigWellResultBranch which contains all the simple pipe
/// sections that make up the well
//==================================================================================================
class RigWellResultFrame
{
public:
    RigWellResultFrame();

    const RigWellResultPoint*       findResultCellWellHeadIncluded( size_t gridIndex, size_t gridCellIndex ) const;
    const RigWellResultPoint*       findResultCellWellHeadExcluded( size_t gridIndex, size_t gridCellIndex ) const;
    std::vector<RigWellResultPoint> allResultPoints() const;

    RigWellResultPoint             wellHeadOrStartCell() const;
    RiaDefines::WellProductionType m_productionType;
    bool                           m_isOpen;
    RigWellResultPoint             m_wellHead;
    QDateTime                      m_timestamp;

    std::vector<RigWellResultBranch> m_wellResultBranches;
};
