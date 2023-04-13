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

#include "RigWellResultBranch.h"

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

    RigWellResultPoint              findResultCellWellHeadIncluded( size_t gridIndex, size_t gridCellIndex ) const;
    RigWellResultPoint              findResultCellWellHeadExcluded( size_t gridIndex, size_t gridCellIndex ) const;
    std::vector<RigWellResultPoint> allResultPoints() const;

    void setIsOpen( bool isOpen );
    void setProductionType( RiaDefines::WellProductionType productionType );
    void setTimestamp( const QDateTime& timestampe );
    void setWellHead( RigWellResultPoint wellHead );

    bool                           isOpen() const;
    RiaDefines::WellProductionType productionType() const;
    QDateTime                      timestamp() const;
    RigWellResultPoint             wellHead() const;
    RigWellResultPoint             wellHeadOrStartCell() const;

    std::vector<RigWellResultBranch> wellResultBranches() const;
    void                             clearWellResultBranches();
    void                             addWellResultBranch( const RigWellResultBranch& wellResultBranch );
    void                             setWellResultBranches( const std::vector<RigWellResultBranch>& wellResultBranches );
    std::vector<RigWellResultPoint>  branchResultPointsFromBranchIndex( size_t index ) const;

private:
    bool                           m_isOpen;
    RiaDefines::WellProductionType m_productionType;
    QDateTime                      m_timestamp;
    RigWellResultPoint             m_wellHead;

    std::vector<RigWellResultBranch> m_wellResultBranches;
};
