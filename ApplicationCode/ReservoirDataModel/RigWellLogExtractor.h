/////////////////////////////////////////////////////////////////////////////////
//
//  Copyright (C) Statoil ASA
//  Copyright (C) Ceetron Solutions AS
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

#include "cvfBase.h"
#include "cvfObject.h"
#include "cvfMath.h"
#include "cvfVector3.h"

#include <vector>
#include "cvfStructGrid.h"

#include "RigWellLogExtractionTools.h"

class RigWellPath;

//==================================================================================================
/// 
//==================================================================================================
class RigWellLogExtractor : public cvf::Object
{
public:
    RigWellLogExtractor(const RigWellPath* wellpath);
    virtual ~RigWellLogExtractor();

    const std::vector<double>&  measuredDepth()     { return m_measuredDepth; }
    const std::vector<double>&  trueVerticalDepth() {return m_trueVerticalDepth;}

    const RigWellPath*          wellPathData()      { return m_wellPath.p();}

protected:
    void populateReturnArrays(std::map<RigMDCellIdxEnterLeaveIntersectionSorterKey, HexIntersectionInfo > &uniqueIntersections);

    std::vector<double>         m_measuredDepth;
    std::vector<double>         m_trueVerticalDepth;
    
    std::vector<cvf::Vec3d>     m_intersections;
    std::vector<size_t>         m_intersectedCells;
    std::vector<cvf::StructGridInterface::FaceType> 
                                m_intersectedCellFaces;

    cvf::cref<RigWellPath>      m_wellPath;
};



