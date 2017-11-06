/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) 2017     Statoil ASA
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

#include <vector>
#include <vector>

#include "cvfBase.h"
#include "cvfVector3.h"
#include "RigSimWellData.h"

class RigSimWellBranchPseudoLengthCalculator
{

public:

    RigSimWellBranchPseudoLengthCalculator (const std::vector< std::vector <cvf::Vec3d> >& pipeBranchesCLCoords,
                                            const std::vector< std::vector <RigWellResultPoint> >& pipeBranchesCellIds);

    const std::vector< std::vector <double> >& pseudoLengthsPrBranch() { return m_pipeBranchesCellMDs; }  

private:
    void calculatePseudoLength(size_t branchIdx, double  startPseudoLengthFromTop);
    std::vector<size_t> findDownStreamBranchIdxs(const RigWellResultPoint& connectionPoint);

private:
    const std::vector< std::vector <RigWellResultPoint> >& m_pipeBranchesCellIds;
    const std::vector< std::vector <cvf::Vec3d> >&          m_pipeBranchesCLCoords;
    std::vector< std::vector <double> >               m_pipeBranchesCellMDs;

};



