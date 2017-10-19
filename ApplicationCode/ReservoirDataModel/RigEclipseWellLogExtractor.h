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

#include "RigWellLogExtractor.h"

class RigEclipseCaseData;
class RigWellPath;
class RigResultAccessor;

struct WellPathCellIntersectionInfo;

namespace cvf {
    class BoundingBox;
}

//==================================================================================================
/// 
//==================================================================================================
class RigEclipseWellLogExtractor : public RigWellLogExtractor
{
public:
    RigEclipseWellLogExtractor(const RigEclipseCaseData* aCase, const RigWellPath* wellpath, const std::string& wellCaseErrorMsgName);

    void                                      curveData(const RigResultAccessor* resultAccessor, std::vector<double>* values );
    const RigEclipseCaseData*                 caseData()     { return m_caseData.p();}

    std::vector<WellPathCellIntersectionInfo> cellIntersectionInfo();
    const std::vector<size_t>&                intersectedCellsGlobIdx();

protected:
    void                                      calculateIntersection();
    std::vector<size_t>                       findCloseCells(const cvf::BoundingBox& bb);

    cvf::cref<RigEclipseCaseData>             m_caseData;
};
