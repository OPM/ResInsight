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
#include <map>

#include "cvfStructGrid.h"

#include "RigWellLogExtractionTools.h"
#include "RigHexIntersectionTools.h"

class RigWellPath;

//==================================================================================================
/// 
//==================================================================================================
class RigWellLogExtractor : public cvf::Object
{
public:
    RigWellLogExtractor(const RigWellPath* wellpath, const std::string& wellCaseErrorMsgName);
    virtual ~RigWellLogExtractor();

    const std::vector<double>&  measuredDepth()     { return m_measuredDepth; }
    const std::vector<double>&  trueVerticalDepth() { return m_trueVerticalDepth; }

    const RigWellPath*          wellPathData()      { return m_wellPath.p();}

protected:
    static void                 insertIntersectionsInMap(const std::vector<HexIntersectionInfo> &intersections,
                                                         cvf::Vec3d p1,
                                                         double md1,
                                                         cvf::Vec3d p2,
                                                         double md2,
                                                         std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo > *uniqueIntersections);

    void                        populateReturnArrays(std::map<RigMDCellIdxEnterLeaveKey, HexIntersectionInfo > &uniqueIntersections);
    void                        appendIntersectionToArrays(double measuredDepth, const HexIntersectionInfo& intersection);

    std::vector<double>         m_measuredDepth;
    std::vector<double>         m_trueVerticalDepth;
    
    std::vector<cvf::Vec3d>     m_intersections;
    std::vector<size_t>         m_intersectedCellsGlobIdx;
    std::vector<cvf::StructGridInterface::FaceType> 
                                m_intersectedCellFaces;

    cvf::cref<RigWellPath>      m_wellPath;

    std::string                 m_wellCaseErrorMsgName;
};



